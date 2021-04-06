/*
 ============================================================================
 Name        : multipart_upload.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "multipart_upload.h"
#include <curl/curl.h>
#include <sys/stat.h>
#include "../cJSON/cJSON.h"
/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

char *extractBucket(const char *uptoken);
char *encodeKey(const char *key, bool hasKey);
const char *get_Qiniu_UpHost(Qiniu_Multipart_PutExtra *extra);

Qiniu_Error init_part(Qiniu_Client *client, const char *bucket, const char *key, bool hasKey, Qiniu_Multipart_PutExtra *extraParam, Qiniu_InitPart_Ret *initPartRet)
{
    Qiniu_Error err;
    char *encodedKey = encodeKey(key, hasKey);
    const char *uphost = get_Qiniu_UpHost(extraParam);
    char *reqUrl = Qiniu_String_Concat(uphost, "/buckets/", bucket, "/objects/", encodedKey,
                                       "/uploads", NULL);

    Qiniu_Json *callRet;
    err = Qiniu_Client_Call(client, &callRet, reqUrl);
    if (err.code == 200)
    {
        const char *uploadId = Qiniu_Json_GetString(callRet, "uploadId", NULL);
        initPartRet->uploadId = Qiniu_String_Dup(uploadId);
    }
    Qiniu_Free(reqUrl);
    Qiniu_Free(encodedKey);
    return err;
}

Qiniu_Error upload_part(Qiniu_Client *client, const char *bucket, const char *key, bool hasKey, const char *uploadId, Qiniu_ReaderAt *reader, Qiniu_Int64 fsize, Qiniu_Multipart_PutExtra *extraParam, Qiniu_UploadParts_Ret *uploadPartsRet)
{
    client->httpMethod = "PUT"; //reset before return
    Qiniu_Error err;
    const char *uphost = get_Qiniu_UpHost(extraParam);
    char *encodedKey = encodeKey(key, hasKey);

    Qiniu_Int64 partSize = extraParam->partSize;
    int totalPartNum = (fsize + partSize - 1) / partSize;
    if (totalPartNum == 0)
    {
        totalPartNum = 1; //even if fsize=0, at least one part
    }
    int lastPart = totalPartNum - 1;

    uploadPartsRet->PartEtag = malloc(totalPartNum * sizeof(*(uploadPartsRet->PartEtag)));
    for (int partNum = 0; partNum < totalPartNum; partNum++)
    {
        int partNumInReq = partNum + 1;
        char partNumStr[20];
        sprintf(partNumStr, "%d", partNumInReq);
        char *reqUrl = Qiniu_String_Concat(uphost, "/buckets/", bucket, "/objects/", encodedKey,
                                           "/uploads/", uploadId, "/", partNumStr, NULL);
        Qiniu_Section section;
        Qiniu_Int64 thisPartOffset = partNum * partSize;
        Qiniu_Int64 thisPartSize = partSize;
        if (partNum == lastPart)
        {
            thisPartSize = fsize - (totalPartNum - 1) * partSize;
        }
        Qiniu_Reader thisPartBody = Qiniu_SectionReader(&section, *reader, (Qiniu_Off_T)thisPartOffset, thisPartSize);
        // body = Qiniu_TeeReader(&tee, body1, md5w);
        Qiniu_Json *root;
        err = Qiniu_Client_CallWithBinary(client, &root, reqUrl, thisPartBody, thisPartSize, "application/octet-stream");
        if (err.code != 200)
        {
            Qiniu_Log_Warn("upload_part: partNum:%d, err:%d, errMsg:%s ", partNumInReq, err.code, err.message);
            Qiniu_Free(reqUrl);
            return err;
        }
        else
        {
            const char *etag = Qiniu_Json_GetString(root, "etag", NULL);
            uploadPartsRet->PartEtag[partNum].etag = Qiniu_String_Dup(etag);
            uploadPartsRet->PartEtag[partNum].partNum = partNumInReq;
        }
        Qiniu_Log_Info("upload_part: uploadid:%s, partNum:%d ", uploadId, partNum);
        Qiniu_Free(reqUrl);
    }
    return err;
}

Qiniu_Error openReader(const char *localFile, Qiniu_File **f, Qiniu_Int64 *fsize)
{

    Qiniu_FileInfo fi;
    Qiniu_Error err = Qiniu_File_Open(f, localFile);
    if (err.code != 200)
    {
        return err;
    }
    err = Qiniu_File_Stat(*f, &fi);
    if (err.code == 200)
    {
        *fsize = Qiniu_FileInfo_Fsize(fi);
    }
    return err;
}
//TODO: support more put policy
Qiniu_Error Qiniu_Multipart_PutWithKey(
    Qiniu_Client *client, Qiniu_Multipart_PutRet *ret,
    const char *uptoken, const char *key, const char *localFile, Qiniu_Multipart_PutExtra *extraParam)
{
    if (extraParam->partSize == 0)
    {
        extraParam->partSize = (4 << 20); //4M
    }
    Qiniu_Error err;
    client->auth = Qiniu_UptokenAuth(uptoken);
    char *bucket = extractBucket(uptoken);

    //step1: init part
    Qiniu_InitPart_Ret initPartRet;
    err = init_part(client, bucket, key, true, extraParam, &initPartRet);
    if (err.code != 200)
    {
        //Qiniu_Multi_Free(bucket,...);
        Qiniu_Log_Warn("initPart err:%d, errMsg:%s ", err.code, err.message);
        return err;
    }
    Qiniu_Log_Info("initPart.uploadId: %s", initPartRet.uploadId);

    //step2: upload part
    Qiniu_Int64 fsize;
    Qiniu_File *f;
    err = openReader(localFile, &f, &fsize);
    if (err.code != 200)
    {
        Qiniu_Log_Warn("openReader err:%d, errMsg:%s ", err.code, err.message);
        return err;
    }
    Qiniu_ReaderAt reader = Qiniu_FileReaderAt(f);
    Qiniu_UploadParts_Ret uploadPartsRet;
    err = upload_part(client, bucket, key, true, initPartRet.uploadId, &reader, fsize, extraParam, &uploadPartsRet);
    if (err.code != 200)
    {
        Qiniu_Log_Warn("upload_part err:%d, errMsg:%s ", err.code, err.message);
        return err;
    }
    Qiniu_Log_Info("uploadParts ok for uploadId: %s", initPartRet.uploadId);
    //step3: complete part
    Qiniu_File_Close(f);
    Qiniu_Free(bucket);
    Qiniu_FreeV2(&(client->auth.self));
    return err;
}

Qiniu_Error Qiniu_Multipart_PutWithoutKey(
    Qiniu_Client *client, Qiniu_Multipart_PutRet *ret,
    const char *uptoken, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_Multipart_PutExtra *extra)
{
    Qiniu_Error err;
    return err;
}

char *extractBucket(const char *uptoken)
{
    //b64decode, unmarshal json uptoken
    //
    //cJSON_Parse(uptoken)
    //TODO:mock first
    return Qiniu_String_Dup("sdk");
}

char *encodeKey(const char *key, bool hasKey)
{
    if (!hasKey)
    {
        char *ret = (char *)malloc(2);
        ret[0] = '~'; //服务端分片上传协议规定,~表示不设置文件名
        ret[1] = '\0';
        return ret;
    }
    return Qiniu_String_Encode(key);
}

const char *get_Qiniu_UpHost(Qiniu_Multipart_PutExtra *extra)
{
    const char *upHost = QINIU_UP_HOST;
    if (extra && extra->upHost != NULL)
    {
        upHost = extra->upHost;
    }
    return upHost;
}