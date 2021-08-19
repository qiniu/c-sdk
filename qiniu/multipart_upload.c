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
#include "recorder.h"
#include "recorder_key.h"
#include "recorder_utils.h"
#include "../cJSON/cJSON.h"
#include "tm.h"
/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

static void Qiniu_Multi_Free(int n, ...);
static char *encodeKey(const char *key, bool hasKey);
static void restoreAuth(Qiniu_Auth *a, Qiniu_Auth backup);
static cJSON *buildJsonMap(int kvNum, const char *(*kvpairs)[2]);
static Qiniu_Error verifyParam(Qiniu_Multipart_PutExtra *param);
static Qiniu_Error openFileReader(const char *fileName, Qiniu_File **f, Qiniu_Int64 *fsize);
static const char *caculatePartMd5(Qiniu_ReaderAt reader, Qiniu_Int64 offset, Qiniu_Int64 partSize);
static Qiniu_Error readMedium(struct Qiniu_Record_Medium *medium, const char **uploadId, Qiniu_UploadPartResp *ret);
static Qiniu_Error writeMedium(struct Qiniu_Record_Medium *medium, const char *uploadId, Qiniu_UploadPartResp *ret);

typedef struct _Qiniu_Multipart_Recorder
{
    Qiniu_Record_Medium *recorderMedium;
    const char *recorderKey;
    Qiniu_Bool toLoadProgresses;
} Qiniu_Multipart_Recorder;

typedef struct
{
    char *uploadId;
} Qiniu_InitPart_Ret;
static void Qiniu_InitPart_Ret_Clean(Qiniu_InitPart_Ret *p)
{
    free(p->uploadId);
}

typedef struct
{
    Qiniu_UploadPartResp *PartsRet;
    int totalPartNum;
} Qiniu_UploadParts_Ret;
static void Qiniu_UploadParts_Ret_Initial(Qiniu_UploadParts_Ret *p, int totalPartNum)
{
    p->totalPartNum = totalPartNum;
    p->PartsRet = calloc(totalPartNum, sizeof(Qiniu_UploadPartResp));
}
static void Qiniu_UploadParts_Ret_Clear(Qiniu_UploadParts_Ret *p, int totalPartNum)
{
    memset(p->PartsRet, 0, totalPartNum * sizeof(Qiniu_UploadPartResp));
}
static void Qiniu_UploadParts_Ret_Clean(Qiniu_UploadParts_Ret *p)
{
    if (p->PartsRet != NULL)
    {
        for (int i = 0; i < p->totalPartNum; i++)
        {
            free(p->PartsRet[i].etag);
            free(p->PartsRet[i].md5);
        }
        free(p->PartsRet);
    }
    p->totalPartNum = 0;
}

static const char *loadProgresses(Qiniu_Multipart_Recorder *recorder, Qiniu_UploadParts_Ret *parts);

static void Qiniu_Multipart_Recorder_Cleanup(Qiniu_Multipart_Recorder *recorder)
{
    if (recorder != NULL)
    {
        if (recorder->recorderMedium != NULL)
        {
            recorder->recorderMedium->close(recorder->recorderMedium);
            recorder->recorderMedium = NULL;
        }
        if (recorder->recorderKey != NULL)
        {
            Qiniu_FreeV2((void **)&recorder->recorderKey);
        }
    }
}

static Qiniu_Error
getTotalPartNum(Qiniu_Int64 fileSize, Qiniu_Int64 partSize, int *totalPartNum)
{
    Qiniu_Error err;
    int num = (fileSize + partSize - 1) / partSize;
    if (num > 10000)
    {
        err.code = 400;
        err.message = "total part num must less then 10000, suggest increase part size";
        return err;
    }
    if (num == 0)
    {
        num = 1; //even if fsize=0, at least one part
    }
    if (totalPartNum != NULL)
    {
        *totalPartNum = num;
    }
    return Qiniu_OK;
}

static Qiniu_Error init_upload(Qiniu_Client *client, const char *bucket, const char *encodedKey, Qiniu_Multipart_PutExtra *extraParam, Qiniu_InitPart_Ret *initPartRet)
{
    Qiniu_Error err;
    const char *uphost = extraParam->upHost;
    char *reqUrl = Qiniu_String_Concat(uphost, "/buckets/", bucket, "/objects/", encodedKey, "/uploads", NULL);

    for (int i = 0; i < extraParam->tryTimes; i++)
    {
        Qiniu_Json *callRet = NULL; //don't cJSON_Delete(callRet), it will be automatically freed on next http request by Qiniu_Client_Call.
        err = Qiniu_Client_Call(client, &callRet, reqUrl);
        if ((err.code / 100 == 5) || (callRet == NULL)) //5xx , net error will retry
        {
            Qiniu_Log_Error("init_upload retry:  %E", err);
            continue;
        }
        else if (err.code != 200)
        {
            Qiniu_Log_Error("init_upload:  %E", err);
            break;
        }

        const char *uploadId = Qiniu_Json_GetString(callRet, "uploadId", NULL);
        initPartRet->uploadId = Qiniu_String_Dup(uploadId);
        break;
    }
    Qiniu_Free(reqUrl);
    return err;
}

Qiniu_Error upload_one_part(Qiniu_Client *client, Qiniu_Multipart_PutExtra *extraParam, const char *reqUrl, int partNum, Qiniu_ReaderAt reader, Qiniu_Int64 partOffset, Qiniu_Int64 partSize, const char *md5str, Qiniu_UploadPartResp *ret)
{
    Qiniu_Error err;
    for (int try = 0; try < extraParam->tryTimes; try++)
    {
        Qiniu_Section section;
        Qiniu_Zero(section);
        Qiniu_Reader thisPartBody = Qiniu_SectionReader(&section, reader, (Qiniu_Off_T)partOffset, partSize);
        Qiniu_Json *callRet = NULL; //don't cJSON_Delete(callRet), it will be automatically freed on next http request by Qiniu_Client_Call.
        err = Qiniu_Client_CallWithMethod(client, &callRet, reqUrl, thisPartBody, partSize, NULL, "PUT", md5str);
        if ((err.code / 100 == 5) || (callRet == NULL)) //5xx,net error will retry
        {
            Qiniu_Log_Error("upload_part retry: partNum:%d, %E", partNum, err);
            continue;
        }
        else if (err.code != 200)
        {
            Qiniu_Log_Error("upload_part: partNum:%d, %E", partNum, err);
            break;
        }

        const char *md5 = Qiniu_Json_GetString(callRet, "md5", NULL);
        const char *etag = Qiniu_Json_GetString(callRet, "etag", NULL);
        ret->etag = Qiniu_String_Dup(etag);
        ret->md5 = Qiniu_String_Dup(md5);
        ret->partNum = partNum;
        // Qiniu_Log_Debug("partNum:%d,remote md5:%s ", partNum, md5);
        break;
    }
    //notify callback
    if ((err.code == 200) && extraParam->notify)
    {
        extraParam->notify(ret);
    }
    if ((err.code != 200) && extraParam->notifyErr)
    {
        extraParam->notifyErr(partNum, err);
    }

    return err;
}

static Qiniu_Error upload_parts(Qiniu_Client *client, const char *bucket, const char *encodedKey, const char *uploadId, Qiniu_ReaderAt *reader, Qiniu_Int64 fsize, int totalPartNum, Qiniu_Multipart_PutExtra *extraParam, Qiniu_Multipart_Recorder *recorder, Qiniu_UploadParts_Ret *uploadPartsRet)
{
    Qiniu_Error err;
    const char *uphost = extraParam->upHost;

    Qiniu_Int64 partSize = extraParam->partSize;
    int lastPart = totalPartNum - 1;
    for (int partNum = 0; partNum < totalPartNum; partNum++)
    {
        if ((uploadPartsRet->PartsRet + partNum)->etag != NULL)
        {
            continue;
        }

        int partNumInReq = partNum + 1; //partNum start from 1
        char partNumStr[10];            //valid partNum ={"1"~"10000"}
        snprintf(partNumStr, 10, "%d", partNumInReq);
        char *reqUrl = Qiniu_String_Concat(uphost, "/buckets/", bucket, "/objects/", encodedKey, "/uploads/", uploadId, "/", partNumStr, NULL);

        Qiniu_Int64 thisPartOffset = partNum * partSize;
        Qiniu_Int64 thisPartSize = partSize;
        if (partNum == lastPart)
        {
            thisPartSize = fsize - (totalPartNum - 1) * partSize;
        }

        const char *md5str = NULL;
        if (extraParam->enableContentMd5)
        {
            md5str = caculatePartMd5(*reader, thisPartOffset, thisPartSize);
            // Qiniu_Log_Debug("partNum:%d, local Md5:%s ", partNumInReq, md5str);
        }
        err = upload_one_part(client, extraParam, reqUrl, partNumInReq, *reader, thisPartOffset, thisPartSize, md5str, &uploadPartsRet->PartsRet[partNum]);

        Qiniu_Multi_Free(2, (void *)reqUrl, (void *)md5str);

        if (err.code != 200)
        {
            Qiniu_UploadParts_Ret_Clean(uploadPartsRet);
            return err;
        }

        if (recorder != NULL && recorder->recorderMedium != NULL)
        {
            err = writeMedium(recorder->recorderMedium, uploadId, &uploadPartsRet->PartsRet[partNum]);
            if (err.code != 200)
            {
                Qiniu_UploadParts_Ret_Clean(uploadPartsRet);
                return err;
            }
        }
    }
    return err;
}

static Qiniu_Error complete_upload(
    Qiniu_Client *client, const char *bucket, const char *encodedKey,
    const char *uploadId, Qiniu_Multipart_PutExtra *extraParam, Qiniu_Multipart_Recorder *recorder,
    Qiniu_UploadParts_Ret *uploadPartsRet, Qiniu_MultipartUpload_Result *completeRet)
{
    //step1: build body
    cJSON *root = cJSON_CreateObject();
    cJSON *parts = cJSON_CreateArray();
    for (int i = 0; i < uploadPartsRet->totalPartNum; i++)
    {
        cJSON *part = cJSON_CreateObject();
        cJSON_AddStringToObject(part, "etag", uploadPartsRet->PartsRet[i].etag);
        cJSON_AddNumberToObject(part, "partNumber", uploadPartsRet->PartsRet[i].partNum);
        cJSON_AddItemToArray(parts, part);
    }
    cJSON_AddItemToObject(root, "parts", parts);
    if (extraParam->mimeType != NULL)
    {
        cJSON_AddStringToObject(root, "mimeType", extraParam->mimeType);
    }
    //add metaData,customVars
    if ((extraParam->xVarsCount > 0) && (extraParam->xVarsList != NULL))
    {
        cJSON *xvarMap = buildJsonMap(extraParam->xVarsCount, extraParam->xVarsList);
        cJSON_AddItemToObject(root, "customVars", xvarMap);
    }
    if ((extraParam->metaCount > 0) && (extraParam->metaList != NULL))
    {
        cJSON *metaMap = buildJsonMap(extraParam->metaCount, extraParam->metaList);
        cJSON_AddItemToObject(root, "metadata", metaMap);
    }

    char *body = cJSON_PrintUnformatted(root);
    // Qiniu_Log_Debug("upload.body:%s ", body);
    cJSON_Delete(root);

    //step2:send req
    Qiniu_Error err;
    char *reqUrl = Qiniu_String_Concat(extraParam->upHost, "/buckets/", bucket, "/objects/", encodedKey, "/uploads/", uploadId, NULL);

    for (int try = 0; try < extraParam->tryTimes; try++)
    {
        Qiniu_Json *callRet = NULL; //don't cJSON_Delete(callRet), it will be automatically freed on next http request by Qiniu_Client_Call.
        err = Qiniu_Client_CallWithBuffer(client, &callRet, reqUrl, body, strlen(body), "application/json");
        if ((err.code / 100 == 5) || (callRet == NULL)) //5xx,net error will retry
        {
            Qiniu_Log_Error("cupload: retry uploadId:%s, %E", uploadId, err);
            continue;
        }
        if (err.code / 100 != 4 && extraParam->recorder != NULL && recorder != NULL && recorder->recorderKey != NULL)
        {
            extraParam->recorder->remove(extraParam->recorder, recorder->recorderKey);
        }
        if (err.code != 200)
        {
            Qiniu_Log_Error("cupload: uploadId:%s, %E", uploadId, err);
            break;
        }

        const char *hash = Qiniu_Json_GetString(callRet, "hash", NULL); //don't free(hash) since no malloc happen.
        const char *key = Qiniu_Json_GetString(callRet, "key", NULL);
        completeRet->hash = Qiniu_String_Dup(hash);
        completeRet->key = Qiniu_String_Dup(key);
        break;
    }
    // Qiniu_Log_Debug("Upload result: uploadid:%s, hash:%s, key:%s ", uploadId, completeRet->hash, completeRet->key);
    Qiniu_Multi_Free(2, (void *)reqUrl, (void *)body);
    return err;
}

static Qiniu_Error _Qiniu_Multipart_Put(
    Qiniu_Client *client, const char *uptoken, const char *key,
    Qiniu_ReaderAt reader, Qiniu_Int64 fsize, Qiniu_Multipart_PutExtra *extraParam, Qiniu_Multipart_Recorder *recorder, Qiniu_MultipartUpload_Result *uploadResult)
{
    Qiniu_Error err;
    err = verifyParam(extraParam);
    if (err.code != 200)
    {
        Qiniu_Log_Error("invalid param %E", err);
        return err;
    }

    const char *bucket;
    if (!Qiniu_Utils_Extract_Bucket(uptoken, NULL, &bucket))
    {
        err.code = 400;
        err.message = "parse uptoken failed";
        return err;
    }

    bool hasKey = (key != NULL);
    char *encodedKey = encodeKey(key, hasKey);
    Qiniu_Auth authBackup = client->auth;
    client->auth = Qiniu_UptokenAuth(uptoken);

    int totalPartNum;
    err = getTotalPartNum(fsize, extraParam->partSize, &totalPartNum);
    if (err.code != 200)
    {
        return err;
    }

    const char *uploadId;
    Qiniu_UploadParts_Ret uploadPartsRet;
    Qiniu_UploadParts_Ret_Initial(&uploadPartsRet, totalPartNum);
    uploadId = loadProgresses(recorder, &uploadPartsRet);

    if (uploadId == NULL)
    {
        //step1: init part
        Qiniu_InitPart_Ret initPartRet;
        err = init_upload(client, bucket, encodedKey, extraParam, &initPartRet);
        if (err.code != 200)
        {
            Qiniu_Log_Error("initUpload %E ", err);
            Qiniu_Multi_Free(2, (void *)bucket, (void *)encodedKey);
            restoreAuth(&client->auth, authBackup);
            Qiniu_UploadParts_Ret_Clear(&uploadPartsRet, totalPartNum);
            return err;
        }
        uploadId = initPartRet.uploadId;
        Qiniu_UploadParts_Ret_Clear(&uploadPartsRet, totalPartNum);
    }

    //step2: upload part
    err = upload_parts(client, bucket, encodedKey, uploadId, &reader, fsize, totalPartNum, extraParam, recorder, &uploadPartsRet);
    if (err.code != 200)
    {
        Qiniu_Log_Error("upload_part %E", err);
        Qiniu_Multi_Free(3, (void *)uploadId, (void *)bucket, (void *)encodedKey);
        restoreAuth(&client->auth, authBackup);
        return err;
    }

    //step3: complete part
    err = complete_upload(client, bucket, encodedKey, uploadId, extraParam, recorder, &uploadPartsRet, uploadResult);
    if (err.code != 200)
    {
        Qiniu_Log_Error("complete_upload %E", err);
    }

    //step4: free memory
    restoreAuth(&client->auth, authBackup);
    Qiniu_Multi_Free(3, (void *)uploadId, (void *)bucket, (void *)encodedKey);
    Qiniu_UploadParts_Ret_Clean(&uploadPartsRet);
    return err;
}

Qiniu_Error Qiniu_Multipart_Put(
    Qiniu_Client *client, const char *uptoken, const char *key,
    Qiniu_ReaderAt reader, Qiniu_Int64 fsize, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *uploadResult)
{
    return _Qiniu_Multipart_Put(client, uptoken, key, reader, fsize, extraParam, NULL, uploadResult);
}

Qiniu_Error Qiniu_Multipart_PutFile(
    Qiniu_Client *client, const char *uptoken, const char *key,
    const char *fileName, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *uploadResult)
{
    Qiniu_File *f = NULL;
    Qiniu_Int64 fsize;
    Qiniu_FileInfo fi;
    Qiniu_Record_Medium medium;
    Qiniu_Multipart_Recorder recorder, *pRecorder = NULL;
    Qiniu_Bool ok;
    Qiniu_Error err;

    err = openFileReader(fileName, &f, &fsize);
    if (err.code != 200)
    {
        Qiniu_Log_Error("openFileReader failed %E", err);
        return err;
    }
    err = Qiniu_File_Stat(f, &fi);
    if (err.code != 200)
    {
        Qiniu_Log_Error("statFile failed %E", err);
        return err;
    }
    Qiniu_ReaderAt reader = Qiniu_FileReaderAt(f);

    if (extraParam->recorder != NULL)
    {
        err = Qiniu_Utils_Generate_RecorderKey(uptoken, "v2", key, fileName, &recorder.recorderKey);
        if (err.code != 200)
        {
            return err;
        }
        err = Qiniu_Utils_Find_Medium(extraParam->recorder, recorder.recorderKey, 1, &medium, &fi, &ok);
        if (err.code != 200)
        {
            return err;
        }
        if (ok)
        {
            recorder.recorderMedium = &medium;
            recorder.toLoadProgresses = Qiniu_True;
        }
        else
        {
            err = Qiniu_Utils_New_Medium(extraParam->recorder, recorder.recorderKey, 1, &medium, &fi);
            if (err.code != 200)
            {
                return err;
            }
            recorder.recorderMedium = &medium;
            recorder.toLoadProgresses = Qiniu_False;
        }
        pRecorder = &recorder;
    }
    err = _Qiniu_Multipart_Put(client, uptoken, key, reader, fsize, extraParam, pRecorder, uploadResult);

    Qiniu_Multipart_Recorder_Cleanup(pRecorder);

    Qiniu_File_Close(f);

    return err;
}

//-------------------------------------------------
void restoreAuth(Qiniu_Auth *a, Qiniu_Auth backup)
{
    free(a->self);
    *a = backup;
}

Qiniu_Error openFileReader(const char *fileName, Qiniu_File **f, Qiniu_Int64 *fsize)
{

    Qiniu_FileInfo fi;
    Qiniu_Error err = Qiniu_File_Open(f, fileName);
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

static const Qiniu_Int64 Min_Part_Size = (1 << 20); //1MB
static const Qiniu_Int64 Max_Part_Size = (1 << 30); //1GB

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
    if (param->tryTimes == 0)
    {
        param->tryTimes = 3;
    }
    if (param->upHost == NULL)
    {
        param->upHost = QINIU_UP_HOST;
    }

    return err;
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

cJSON *buildJsonMap(int kvNum, const char *(*kvpairs)[2])
{
    cJSON *map = cJSON_CreateObject();
    for (int i = 0; i < kvNum; i++)
    {
        cJSON_AddStringToObject(map, kvpairs[i][0], kvpairs[i][1]);
    }
    return map;
}
//-----------------------------------memory free helper func
void Qiniu_Multi_Free(int n, ...)
{
    void *p = NULL;
    va_list v1;
    va_start(v1, n);
    for (int i = 0; i < n; i++)
    {
        p = va_arg(v1, void *);
        free(p);
    }
    va_end(v1);
}

Qiniu_Error readMedium(struct Qiniu_Record_Medium *medium, const char **uploadId, Qiniu_UploadPartResp *ret)
{
    Qiniu_Error err;
#define BUFFER_SIZE (1 << 22)
    size_t haveRead;
    char buf[BUFFER_SIZE];
    err = medium->readEntry(medium, buf, BUFFER_SIZE, &haveRead);
    if (err.code != 200)
    {
        medium->close(medium);
        return err;
    }
    if (haveRead >= BUFFER_SIZE)
    {
        medium->close(medium);
        err.code = 500;
        err.message = "recorder entry is too large";
        return err;
    }
    cJSON *blockInfo = cJSON_Parse(buf);
    ret->partNum = (int)Qiniu_Json_GetInt64(blockInfo, "partNum", -1);
    ret->md5 = strdup(Qiniu_Json_GetString(blockInfo, "md5", NULL));
    ret->etag = strdup(Qiniu_Json_GetString(blockInfo, "etag", NULL));
    if (uploadId != NULL)
    {
        *uploadId = strdup(Qiniu_Json_GetString(blockInfo, "uploadId", NULL));
    }
    cJSON_Delete(blockInfo);
    return Qiniu_OK;
#undef BUFFER_SIZE
}

Qiniu_Error writeMedium(struct Qiniu_Record_Medium *medium, const char *uploadId, Qiniu_UploadPartResp *ret)
{
    Qiniu_Error err;
    cJSON *blockInfo = cJSON_CreateObject();
    cJSON_AddItemToObject(blockInfo, "partNum", cJSON_CreateNumber(ret->partNum));
    cJSON_AddItemToObject(blockInfo, "md5", cJSON_CreateString(ret->md5));
    cJSON_AddItemToObject(blockInfo, "etag", cJSON_CreateString(ret->etag));
    cJSON_AddItemToObject(blockInfo, "uploadId", cJSON_CreateString(uploadId));
    char *blockInfoJson = cJSON_PrintUnformatted(blockInfo);
    cJSON_Delete(blockInfo);
    if (blockInfoJson == NULL)
    {
        err.code = 400;
        err.message = "cJSON_PrintUnformatted() error";
    }
    err = medium->writeEntry(medium, blockInfoJson, NULL);
    free(blockInfoJson);
    if (err.code != 200)
    {
        return err;
    }

    return Qiniu_OK;
}

const char *loadProgresses(Qiniu_Multipart_Recorder *recorder, Qiniu_UploadParts_Ret *parts)
{
    Qiniu_Error err;
    Qiniu_Bool hasNext;
    int blkIdx;
    Qiniu_UploadPartResp ret;
    const char *uploadId = NULL;
    if (recorder != NULL && recorder->recorderMedium != NULL && recorder->toLoadProgresses)
    {
        for (;;)
        {
            err = recorder->recorderMedium->hasNextEntry(recorder->recorderMedium, &hasNext);
            if (err.code != 200)
            {
                return NULL;
            }
            if (hasNext)
            {
                if (uploadId == NULL)
                {
                    err = readMedium(recorder->recorderMedium, &uploadId, &ret);
                }
                else
                {
                    err = readMedium(recorder->recorderMedium, NULL, &ret);
                }

                if (err.code != 200)
                {
                    return NULL;
                }
                *(parts->PartsRet + ret.partNum - 1) = ret;
            }
            else
            {
                break;
            }
        }
    }

    return uploadId;
}
