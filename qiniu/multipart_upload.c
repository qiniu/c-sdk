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
#include "tm.h"
/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

char *extractBucket(const char *uptoken);
char *encodeKey(const char *key, bool hasKey);
const char *caculatePartMd5(Qiniu_ReaderAt reader, Qiniu_Int64 offset, Qiniu_Int64 partSize);
void Qiniu_Multi_Free(int n, ...);

typedef struct
{
    char *uploadId;
} Qiniu_InitPart_Ret;
void Qiniu_InitPart_Ret_Clean(Qiniu_InitPart_Ret *p)
{
    free(p->uploadId);
}
typedef struct
{
    struct
    {
        char *etag;
        int partNum;
    } * PartEtag;

    int totalPartNum;

} Qiniu_UploadParts_Ret;
void Qiniu_UploadParts_Ret_Clean(Qiniu_UploadParts_Ret *p)
{
    if (p->PartEtag != NULL)
    {
        for (int i = 0; i < p->totalPartNum; i++)
        {
            free(p->PartEtag[i].etag);
        }
        free(p->PartEtag);
    }
    p->totalPartNum = 0;
}

Qiniu_Error init_upload(Qiniu_Client *client, const char *bucket, const char *encodedKey, Qiniu_Multipart_PutExtra *extraParam, Qiniu_InitPart_Ret *initPartRet)
{
    Qiniu_Error err;
    const char *uphost = client->upHost;
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
    return err;
}

Qiniu_Error upload_parts(Qiniu_Client *client, const char *bucket, const char *encodedKey, const char *uploadId, Qiniu_ReaderAt *reader, Qiniu_Int64 fsize, Qiniu_Multipart_PutExtra *extraParam, Qiniu_UploadParts_Ret *uploadPartsRet)
{
    Qiniu_Error err;
    const char *uphost = client->upHost;

    Qiniu_Int64 partSize = extraParam->partSize;
    int totalPartNum = (fsize + partSize - 1) / partSize;
    if (totalPartNum == 0)
    {
        totalPartNum = 1; //even if fsize=0, at least one part
    }
    int lastPart = totalPartNum - 1;

    uploadPartsRet->totalPartNum = totalPartNum;
    uploadPartsRet->PartEtag = malloc(totalPartNum * sizeof(*(uploadPartsRet->PartEtag)));
    for (int partNum = 0; partNum < totalPartNum; partNum++)
    {
        int partNumInReq = partNum + 1; //partNum start from 1
        char partNumStr[10];            //valid partNum ={"1"~"10000"}
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
        const char *md5str;
        if (extraParam->enableContentMd5)
        {
            md5str = caculatePartMd5(*reader, thisPartOffset, thisPartSize);
            Qiniu_Log_Debug("partNum:%d, local Md5:%s ", partNum, md5str);
        }

        Qiniu_Json *result;
        err = Qiniu_Client_CallWithMethod(client, &result, reqUrl, thisPartBody, thisPartSize, "application/octet-stream", "PUT", md5str);
        Qiniu_Free(reqUrl);

        if (err.code != 200)
        {
            Qiniu_Log_Error("upload_part: partNum:%d, err:%d, errMsg:%s ", partNumInReq, err.code, err.message);
            Qiniu_UploadParts_Ret_Clean(uploadPartsRet);
            return err;
        }
        else
        {
            Qiniu_Log_Debug("partNum:%d,remote md5:%s ", partNum, Qiniu_Json_GetString(result, "md5", NULL));
            const char *etag = Qiniu_Json_GetString(result, "etag", NULL);
            uploadPartsRet->PartEtag[partNum].etag = Qiniu_String_Dup(etag);
            uploadPartsRet->PartEtag[partNum].partNum = partNumInReq;
        }
        Qiniu_Log_Debug("upload_part: uploadid:%s, partNum:%d ", uploadId, partNum);
    }
    return err;
}

Qiniu_Error complete_upload(Qiniu_Client *client, const char *bucket, const char *encodedKey, const char *uploadId, Qiniu_Multipart_PutExtra *extraParam, Qiniu_UploadParts_Ret *uploadPartsRet, Qiniu_MultipartUpload_Result *completeRet)
{
    //step1: build body
    cJSON *root = cJSON_CreateObject();
    cJSON *parts = cJSON_CreateArray();
    for (int i = 0; i < uploadPartsRet->totalPartNum; i++)
    {
        cJSON *part = cJSON_CreateObject();
        cJSON_AddStringToObject(part, "etag", uploadPartsRet->PartEtag[i].etag);
        cJSON_AddNumberToObject(part, "partNumber", uploadPartsRet->PartEtag[i].partNum);
        cJSON_AddItemToArray(parts, part);
    }
    cJSON_AddItemToObject(root, "parts", parts);
    cJSON_AddStringToObject(root, "mimeType", extraParam->mimeType);
    char *body = cJSON_PrintUnformatted(root);
    Qiniu_Log_Debug("upload.body:%s ", body);
    cJSON_Delete(root);

    //step2:send req
    Qiniu_Error err;
    char *reqUrl = Qiniu_String_Concat(client->upHost, "/buckets/", bucket, "/objects/", encodedKey, "/uploads/", uploadId, NULL);
    Qiniu_Json *result;
    err = Qiniu_Client_CallWithBuffer(client, &result, reqUrl, body, strlen(body), "application/json");
    if (err.code != 200)
    {
        Qiniu_Log_Error("cupload: uploadId:%s, err:%d, errMsg:%s ", uploadId, err.code, err.message);
        Qiniu_Free(reqUrl);
        Qiniu_Free(body);
        return err;
    }
    else
    {
        const char *hash = Qiniu_Json_GetString(result, "hash", NULL);
        const char *key = Qiniu_Json_GetString(result, "key", NULL);
        completeRet->hash = Qiniu_String_Dup(hash);
        completeRet->key = Qiniu_String_Dup(key);
    }
    Qiniu_Log_Debug("Upload result: uploadid:%s, hash:%s, key:%s ", uploadId, completeRet->hash, completeRet->key);
    Qiniu_Free(body);
    Qiniu_Free(reqUrl);
    return err;
}

Qiniu_Error openFileReader(const char *localFile, Qiniu_File **f, Qiniu_Int64 *fsize)
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

const Qiniu_Int64 Min_Part_Size = (1 << 20); //1MB
const Qiniu_Int64 Max_Part_Size = (1 << 30); //1GB

Qiniu_Error verifyParam(Qiniu_Multipart_PutExtra *param)
{
    Qiniu_Error err = {200, ""};
    if (param->partSize == 0)
    {
        param->partSize = (4 << 20); //4M
    }
    if ((param->partSize < Min_Part_Size) || (param->partSize > Max_Part_Size)) //valid part size: 1MB~1GB
    {
        err.code = 400;
        err.message = "partSize must between 1MB and 1GB";
        return err;
    }
    return err;
}
//TODO: support more put policy
Qiniu_Error Qiniu_Multipart_PutFile(Qiniu_Client *client, const char *uptoken, const char *key,
                                    const char *localFile, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *uploadResult)
{
    Qiniu_Error err;
    err = verifyParam(extraParam);
    if (err.code != 200)
    {
        Qiniu_Log_Error("invalid param err:%d, errMsg:%s", err.code, err.message);
        return err;
    }
    Qiniu_Auth authBackup = client->auth;
    client->auth = Qiniu_UptokenAuth(uptoken);
    char *bucket = extractBucket(uptoken);
    if (bucket == NULL)
    {
        err.code = 400;
        err.message = "invalid uptoken,parse uptoken failed";
        Qiniu_Multi_Free(1, (void *)client->auth.self);
        return err;
    }

    bool hasKey = (key != NULL);
    char *encodedKey = encodeKey(key, hasKey);
    //step1: init part
    Qiniu_InitPart_Ret initPartRet;
    err = init_upload(client, bucket, encodedKey, extraParam, &initPartRet);
    if (err.code != 200)
    {
        Qiniu_Log_Error("initUpload err:%d, errMsg:%s ", err.code, err.message);
        Qiniu_Multi_Free(3, (void *)client->auth.self, (void *)bucket, (void *)encodedKey);
        return err;
    }

    //step2: upload part
    Qiniu_Int64 fsize;
    Qiniu_File *f;
    err = openFileReader(localFile, &f, &fsize);
    if (err.code != 200)
    {
        Qiniu_Log_Error("openFileReader err:%d, errMsg:%s ", err.code, err.message);
        Qiniu_Multi_Free(3, (void *)client->auth.self, (void *)bucket, (void *)encodedKey);
        Qiniu_InitPart_Ret_Clean(&initPartRet);
        return err;
    }
    const char *uploadId = initPartRet.uploadId;
    Qiniu_UploadParts_Ret uploadPartsRet;
    Qiniu_ReaderAt reader = Qiniu_FileReaderAt(f);
    err = upload_parts(client, bucket, encodedKey, uploadId, &reader, fsize, extraParam, &uploadPartsRet);
    Qiniu_File_Close(f);

    if (err.code != 200)
    {
        Qiniu_Log_Error("upload_part err:%d, errMsg:%s ", err.code, err.message);
        Qiniu_Multi_Free(3, (void *)client->auth.self, (void *)bucket, (void *)encodedKey);
        Qiniu_InitPart_Ret_Clean(&initPartRet);
        return err;
    }

    //step3: complete part
    err = complete_upload(client, bucket, encodedKey, uploadId, extraParam, &uploadPartsRet, uploadResult);
    if (err.code != 200)
    {
        Qiniu_Log_Error("complete_upload err:%d, errMsg:%s ", err.code, err.message);
    }
    else
    {
        Qiniu_Log_Debug("Upload ok for uploadId: %s", uploadId);
    }

    //step4: free memory
    Qiniu_Multi_Free(3, (void *)client->auth.self, (void *)bucket, (void *)encodedKey);
    Qiniu_InitPart_Ret_Clean(&initPartRet);
    Qiniu_UploadParts_Ret_Clean(&uploadPartsRet);
    client->auth = authBackup;
    return err;
}

char *extractBucket(const char *uptoken)
{
    //uptoken=<ak>:<signedData>:<policyDataBase64>
    //extract put policy str from uptoken
    //step1 decode base64
    int hitCount = 0;
    int hitIndex = -1;
    int len = strlen(uptoken);

    for (int i = 0; i < len; i++)
    {
        if (uptoken[i] == ':')
        {
            hitCount++;
            hitIndex = i;
        }
    }
    if ((hitCount != 2) || ((hitIndex + 1) == len))
    {
        Qiniu_Log_Error("invalid uptoken,should contain exactly two colon");
        return NULL;
    }
    const char *policyB64Data = uptoken + hitIndex + 1;
    char *policyData = Qiniu_String_Decode(policyB64Data);

    //step2 parse json
    cJSON *policy = cJSON_Parse(policyData);
    const char *scope = Qiniu_Json_GetString(policy, "scope", NULL);

    //step3 split bucketname from scope=<bucket>:<xx>
    len = strlen(scope);
    hitIndex = len;
    for (int i = 0; i < len; i++)
    {
        if (scope[i] == ':')
        {
            hitIndex = i;
            break;
        }
    }

    char *bucket = strndup(scope, hitIndex);
    cJSON_Delete(policy);
    Qiniu_Free(policyData);
    Qiniu_Log_Debug("scope.bucket:%s ", bucket);
    return bucket;
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

const char *caculatePartMd5(Qiniu_ReaderAt reader, Qiniu_Int64 offset, Qiniu_Int64 partSize)
{
    Qiniu_Section section;
    Qiniu_Reader partReader = Qiniu_SectionReader(&section, reader, (Qiniu_Off_T)offset, partSize);
    return Qiniu_MD5_HexStr_From_Reader(partReader);
}

//-----------------------------------
void Qiniu_Multi_Free(int n, ...)
{
    void *p;
    va_list v1;
    va_start(v1, n);
    for (int i = 0; i < n; i++)
    {
        p = va_arg(v1, void *);
        free(p);
    }
    va_end(v1);
}

typedef struct
{
    void *ptrList[200];
    int count;
} MemoryDeleteQueue;
void PUSH_MEM_TO_DELETE_QUEUE(MemoryDeleteQueue *queue, void *p)
{
    queue->ptrList[queue->count++] = p;
}
void FREE_MEM_QUEUE(MemoryDeleteQueue *queue)
{
    for (int i = 0; i < queue->count; i++)
    {
        free(queue->ptrList[i]);
    }
    queue->count = 0;
}