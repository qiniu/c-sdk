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
#include "private/region.h"
#include "private/code.h"
/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

static void Qiniu_Multi_Free(int n, ...);
static char *encodeKey(const char *key, bool hasKey);
static void restoreAuth(Qiniu_Auth *a, Qiniu_Auth backup);
static cJSON *buildJsonMap(int kvNum, const char *(*kvpairs)[2]);
static Qiniu_Error verifyParam(Qiniu_Client *client, const char *accessKey, const char *bucketName, Qiniu_Multipart_PutExtra *param);
static Qiniu_Error openFileReader(const char *fileName, Qiniu_File **f);
static const char *caculatePartMd5(Qiniu_ReaderAt reader, Qiniu_Int64 offset, Qiniu_Int64 partSize);

typedef struct _Qiniu_Multipart_Recorder
{
    Qiniu_Record_Medium *recorderMedium;
    const char *recorderKey;
    Qiniu_Bool toLoadProgresses;
} Qiniu_Multipart_Recorder;

typedef struct
{
    char *uploadId;
    Qiniu_Uint64 expireAt;
} Qiniu_InitPart_Ret;

typedef struct
{
    Qiniu_UploadPartResp *PartsRet;
    int totalPartNum;
} Qiniu_UploadParts_Ret;

struct _Qiniu_Progress_Callback_Data
{
    size_t base, totalSize, previousUlNow;
    void (*callback)(size_t, size_t);
};

static Qiniu_Error readMedium(struct Qiniu_Record_Medium *medium, char **uploadId, Qiniu_Uint64 *expireAt, Qiniu_UploadPartResp *ret);
static Qiniu_Error writeMedium(struct Qiniu_Record_Medium *medium, const Qiniu_InitPart_Ret *, const Qiniu_UploadPartResp *);
static Qiniu_Error initializeRecorder(Qiniu_Multipart_PutExtra *param, const char *uptoken, const char *key, const char *fileName, Qiniu_FileInfo *fi, Qiniu_Record_Medium *medium, Qiniu_Multipart_Recorder *recorder);
static Qiniu_Error reinitializeRecorder(Qiniu_Multipart_PutExtra *param, Qiniu_FileInfo *fi, Qiniu_Multipart_Recorder *recorder);
static Qiniu_Error loadProgresses(Qiniu_Multipart_Recorder *recorder, Qiniu_InitPart_Ret *initPartRet, Qiniu_UploadParts_Ret *parts, Qiniu_Multipart_PutExtra *param, Qiniu_FileInfo *fi);

static void Qiniu_InitPart_Ret_Clean(Qiniu_InitPart_Ret *p)
{
    Qiniu_FreeV2((void **)&p->uploadId);
    p->expireAt = 0;
}
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
        num = 1; // even if fsize=0, at least one part
    }
    if (totalPartNum != NULL)
    {
        *totalPartNum = num;
    }
    return Qiniu_OK;
}

static Qiniu_Error init_upload(Qiniu_Client *client, const char *bucket, const char *encodedKey, Qiniu_Multipart_PutExtra *extraParam, Qiniu_InitPart_Ret *initPartRet)
{
    Qiniu_Error err = Qiniu_OK;
    const char *const *upHosts;
    const char *upHost, *reqUrl;
    size_t upHostsCount;
    Qiniu_Json *callRet;

    if (extraParam->upHost != NULL)
    {
        upHosts = &extraParam->upHost;
        upHostsCount = 1;
    }
    else
    {
        upHosts = extraParam->upHosts;
        upHostsCount = extraParam->upHostsCount;
    }

    for (int i = 0; i < extraParam->tryTimes && i <= client->hostsRetriesMax; i++)
    {
        upHost = upHosts[i % upHostsCount];
        reqUrl = Qiniu_String_Concat(upHost, "/buckets/", bucket, "/objects/", encodedKey, "/uploads", NULL);

        callRet = NULL; // don't cJSON_Delete(callRet), it will be automatically freed on next http request by Qiniu_Client_Call.
        err = Qiniu_Client_Call(client, &callRet, reqUrl);
        Qiniu_Free((void *)reqUrl);
        if (err.code == 200)
        {
            if (initPartRet != NULL)
            {
                initPartRet->uploadId = Qiniu_String_Dup(Qiniu_Json_GetString(callRet, "uploadId", NULL));
                initPartRet->expireAt = Qiniu_Json_GetUInt64(callRet, "expireAt", 0);
            }
            break;
        }
        else if (_Qiniu_Should_Retry(err.code) == QINIU_DONT_RETRY)
        {
            break;
        }
    }
    return err;
}

static int _Qiniu_Progress_Callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    struct _Qiniu_Progress_Callback_Data *data = (struct _Qiniu_Progress_Callback_Data *)clientp;
    if (data->previousUlNow != (size_t)ulnow)
    {
        data->callback((size_t)data->totalSize, data->base + (size_t)ulnow);
        data->previousUlNow = (size_t)ulnow;
    }
    return 0;
}

Qiniu_Error upload_one_part(Qiniu_Client *client, Qiniu_Multipart_PutExtra *extraParam, const char *path,
                            int partNum, Qiniu_ReaderAt reader, Qiniu_Int64 partOffset, Qiniu_Int64 partSize, const char *md5str,
                            Qiniu_UploadPartResp *ret, struct _Qiniu_Progress_Callback_Data *progressCallback)
{
    Qiniu_Error err = Qiniu_OK;
    const char *const *upHosts;
    size_t upHostsCount;
    int (*callback)(void *, double, double, double, double) = NULL;
    void *callbackData = NULL;
    if (progressCallback != NULL && progressCallback->callback != NULL)
    {
        callback = _Qiniu_Progress_Callback;
        callbackData = (void *)progressCallback;
    }

    if (extraParam->upHost != NULL)
    {
        upHosts = &extraParam->upHost;
        upHostsCount = 1;
    }
    else
    {
        upHosts = extraParam->upHosts;
        upHostsCount = extraParam->upHostsCount;
    }
    for (int tries = 0; tries < extraParam->tryTimes && tries <= client->hostsRetriesMax; tries++)
    {
        Qiniu_Section section;
        Qiniu_Zero(section);
        Qiniu_Reader thisPartBody = Qiniu_SectionReader(&section, reader, (Qiniu_Off_T)partOffset, partSize);
        Qiniu_Json *callRet = NULL; // don't cJSON_Delete(callRet), it will be automatically freed on next http request by Qiniu_Client_Call.
        const char *upHost = upHosts[tries % upHostsCount];
        const char *reqUrl = Qiniu_String_Concat(upHost, path, NULL);
        err = Qiniu_Client_CallWithMethodAndProgressCallback(client, &callRet, reqUrl, thisPartBody, partSize, NULL, "PUT", md5str, callback, callbackData);
        Qiniu_Free((void *)reqUrl);
        if (err.code == 200)
        {
            if (ret != NULL)
            {
                const char *md5 = Qiniu_Json_GetString(callRet, "md5", NULL);
                const char *etag = Qiniu_Json_GetString(callRet, "etag", NULL);
                ret->etag = Qiniu_String_Dup(etag);
                ret->md5 = Qiniu_String_Dup(md5);
                ret->partNum = partNum;
            }
            break;
        }
        else if (_Qiniu_Should_Retry(err.code) == QINIU_DONT_RETRY)
        {
            break;
        }
    }
    // notify callback
    if (err.code == 200 && extraParam->notify)
    {
        extraParam->notify(ret);
    }
    else if (err.code != 200 && extraParam->notifyErr)
    {
        extraParam->notifyErr(partNum, err);
    }

    return err;
}

static Qiniu_Error upload_parts(Qiniu_Client *client, const char *bucket, const char *encodedKey, const Qiniu_InitPart_Ret *initParts, Qiniu_ReaderAt *reader, Qiniu_Int64 fsize, int totalPartNum, Qiniu_Multipart_PutExtra *extraParam, Qiniu_Multipart_Recorder *recorder, Qiniu_UploadParts_Ret *uploadPartsRet)
{
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Int64 partSize = extraParam->partSize;
    const int lastPart = totalPartNum - 1;
    struct _Qiniu_Progress_Callback_Data progressCallbackData;
    Qiniu_Zero(progressCallbackData);

    if (extraParam->uploadingProgress != NULL)
    {
        progressCallbackData.callback = extraParam->uploadingProgress;
        progressCallbackData.totalSize = (size_t)fsize;
    }

    for (int partNum = 0; partNum < totalPartNum; partNum++)
    {
        Qiniu_Int64 thisPartOffset = partNum * partSize;
        Qiniu_Int64 thisPartSize = partSize;
        if (partNum == lastPart)
        {
            thisPartSize = fsize - (totalPartNum - 1) * partSize;
        }

        if ((uploadPartsRet->PartsRet + partNum)->etag == NULL)
        {
            const int partNumInReq = partNum + 1; // partNum start from 1
            char partNumStr[10];                  // valid partNum ={"1"~"10000"}
            snprintf(partNumStr, 10, "%d", partNumInReq);
            const char *path = Qiniu_String_Concat("/buckets/", bucket, "/objects/", encodedKey, "/uploads/", initParts->uploadId, "/", partNumStr, NULL);

            const char *md5str = NULL;
            if (extraParam->enableContentMd5)
            {
                md5str = caculatePartMd5(*reader, thisPartOffset, thisPartSize);
                // Qiniu_Log_Debug("partNum:%d, local Md5:%s ", partNumInReq, md5str);
            }
            err = upload_one_part(client, extraParam, path, partNumInReq, *reader, thisPartOffset, thisPartSize, md5str, &uploadPartsRet->PartsRet[partNum], &progressCallbackData);
            Qiniu_Multi_Free(2, (void *)path, (void *)md5str);

            if (err.code != 200)
            {
                return err;
            }
            if (recorder != NULL && recorder->recorderMedium != NULL)
            {
                err = writeMedium(recorder->recorderMedium, initParts, &uploadPartsRet->PartsRet[partNum]);
                if (err.code != 200)
                {
                    return err;
                }
            }
        }
        progressCallbackData.base += thisPartSize;
    }
    return err;
}

static Qiniu_Error complete_upload(
    Qiniu_Client *client, const char *bucket, const char *encodedKey,
    Qiniu_InitPart_Ret *initPartsRet, Qiniu_Multipart_PutExtra *extraParam, Qiniu_Multipart_Recorder *recorder,
    Qiniu_UploadParts_Ret *uploadPartsRet, Qiniu_MultipartUpload_Result *completeRet)
{
    // step1: build body
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
    // add metaData,customVars
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

    // step2:send req
    Qiniu_Error err = Qiniu_OK;
    const char *const *upHosts;
    size_t upHostsCount;

    if (extraParam->upHost != NULL)
    {
        upHosts = &extraParam->upHost;
        upHostsCount = 1;
    }
    else
    {
        upHosts = extraParam->upHosts;
        upHostsCount = extraParam->upHostsCount;
    }

    for (int tries = 0; tries < extraParam->tryTimes && tries <= client->hostsRetriesMax; tries++)
    {
        const char *upHost = upHosts[tries % upHostsCount];
        const char *reqUrl = Qiniu_String_Concat(upHost, "/buckets/", bucket, "/objects/", encodedKey, "/uploads/", initPartsRet->uploadId, NULL);
        Qiniu_Json *callRet = NULL; // don't cJSON_Delete(callRet), it will be automatically freed on next http request by Qiniu_Client_Call.
        err = Qiniu_Client_CallWithBuffer(client, &callRet, reqUrl, body, strlen(body), "application/json");
        Qiniu_Free((void *)reqUrl);

        if (err.code == 200)
        {
            if (extraParam->recorder != NULL && recorder != NULL && recorder->recorderKey != NULL)
            {
                extraParam->recorder->remove(extraParam->recorder, recorder->recorderKey);
            }
            if (callRet != NULL)
            {
                const char *hash = Qiniu_Json_GetString(callRet, "hash", NULL); // don't free(hash) since no malloc happen.
                const char *key = Qiniu_Json_GetString(callRet, "key", NULL);
                completeRet->hash = Qiniu_String_Dup(hash);
                completeRet->key = Qiniu_String_Dup(key);
            }
            break;
        }
        else if (_Qiniu_Should_Retry(err.code) == QINIU_DONT_RETRY)
        {
            if (err.code / 100 != 4 && extraParam->recorder != NULL && recorder != NULL && recorder->recorderKey != NULL)
            {
                extraParam->recorder->remove(extraParam->recorder, recorder->recorderKey);
            }
            break;
        }
    }
    // Qiniu_Log_Debug("Upload result: uploadid:%s, hash:%s, key:%s ", uploadId, completeRet->hash, completeRet->key);
    Qiniu_Free((void *)body);
    return err;
}

static Qiniu_Error _Qiniu_Multipart_Put(
    Qiniu_Client *client, const char *uptoken, const char *key,
    Qiniu_ReaderAt reader, Qiniu_Int64 fsize, Qiniu_FileInfo *fi, Qiniu_Multipart_PutExtra *extraParam, Qiniu_Multipart_Recorder *recorder, Qiniu_MultipartUpload_Result *uploadResult)
{
    Qiniu_Error err;

    const char *bucket = NULL, *accessKey = NULL;
    if (!Qiniu_Utils_Extract_Bucket(uptoken, &accessKey, &bucket))
    {
        err.code = 400;
        err.message = "parse uptoken failed";
        return err;
    }

    err = verifyParam(client, accessKey, bucket, extraParam);
    if (err.code != 200)
    {
        Qiniu_Multi_Free(2, (void *)accessKey, (void *)bucket);
        Qiniu_Log_Error("invalid param %E", err);
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

    Qiniu_InitPart_Ret initPartRet = {NULL, 0};
    Qiniu_UploadParts_Ret uploadPartsRet = {NULL, 0};
    Qiniu_UploadParts_Ret_Initial(&uploadPartsRet, totalPartNum);
    if (fi != NULL)
    {
        loadProgresses(recorder, &initPartRet, &uploadPartsRet, extraParam, fi);
    }

    Qiniu_Bool mustInitialize = initPartRet.uploadId == NULL;
    if (mustInitialize == Qiniu_True)
    {
    reinit:
        // step1: init part
        err = init_upload(client, bucket, encodedKey, extraParam, &initPartRet);
        if (err.code != 200)
        {
            Qiniu_Log_Error("initUpload %E ", err);
            Qiniu_Multi_Free(2, (void *)bucket, (void *)encodedKey);
            restoreAuth(&client->auth, authBackup);
            Qiniu_UploadParts_Ret_Clear(&uploadPartsRet, totalPartNum);
            return err;
        }
        Qiniu_UploadParts_Ret_Clear(&uploadPartsRet, totalPartNum);
    }

    // step2: upload part
    err = upload_parts(client, bucket, encodedKey, &initPartRet, &reader, fsize, totalPartNum, extraParam, recorder, &uploadPartsRet);
    if (err.code == 612 && mustInitialize == Qiniu_False)
    {
        Qiniu_Log_Error("reinitUpload %E ", err);
        Qiniu_InitPart_Ret_Clean(&initPartRet);
        Qiniu_UploadParts_Ret_Clear(&uploadPartsRet, totalPartNum);
        mustInitialize = Qiniu_True;
        goto reinit;
    }
    if (err.code != 200)
    {
        Qiniu_Log_Error("upload_part %E", err);
        Qiniu_Multi_Free(2, (void *)bucket, (void *)encodedKey);
        Qiniu_InitPart_Ret_Clean(&initPartRet);
        Qiniu_UploadParts_Ret_Clean(&uploadPartsRet);
        restoreAuth(&client->auth, authBackup);
        return err;
    }

    // step3: complete part
    err = complete_upload(client, bucket, encodedKey, &initPartRet, extraParam, recorder, &uploadPartsRet, uploadResult);
    if (err.code == 612 && mustInitialize == Qiniu_False)
    {
        Qiniu_Log_Error("reinitUpload %E ", err);
        Qiniu_InitPart_Ret_Clean(&initPartRet);
        Qiniu_UploadParts_Ret_Clear(&uploadPartsRet, totalPartNum);
        mustInitialize = Qiniu_True;
        goto reinit;
    }
    if (err.code != 200)
    {
        Qiniu_Log_Error("complete_upload %E", err);
    }

    // step4: free memory
    restoreAuth(&client->auth, authBackup);
    Qiniu_Multi_Free(3, (void *)accessKey, (void *)bucket, (void *)encodedKey);
    Qiniu_InitPart_Ret_Clean(&initPartRet);
    Qiniu_UploadParts_Ret_Clean(&uploadPartsRet);
    return err;
}

Qiniu_Error Qiniu_Multipart_Put(
    Qiniu_Client *client, const char *uptoken, const char *key,
    Qiniu_ReaderAt reader, Qiniu_Int64 fsize, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *uploadResult)
{
    return _Qiniu_Multipart_Put(client, uptoken, key, reader, fsize, NULL, extraParam, NULL, uploadResult);
}

Qiniu_Error Qiniu_Multipart_PutFile(
    Qiniu_Client *client, const char *uptoken, const char *key,
    const char *fileName, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *uploadResult)
{
    Qiniu_File *f = NULL;
    Qiniu_FileInfo fi;
    Qiniu_Record_Medium medium;
    Qiniu_Multipart_Recorder recorder = {NULL, NULL, Qiniu_False}, *pRecorder = NULL;

    Qiniu_Error err = openFileReader(fileName, &f);
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

    err = initializeRecorder(extraParam, uptoken, key, fileName, &fi, &medium, &recorder);
    if (err.code != 200)
    {
        Qiniu_Log_Error("initializeRecorder failed %E", err);
        return err;
    }
    if (recorder.recorderMedium != NULL)
    {
        pRecorder = &recorder;
    }
    err = _Qiniu_Multipart_Put(client, uptoken, key, reader, Qiniu_FileInfo_Fsize(fi), &fi, extraParam, pRecorder, uploadResult);

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

Qiniu_Error openFileReader(const char *fileName, Qiniu_File **f)
{

    Qiniu_FileInfo fi;
    Qiniu_Error err = Qiniu_File_Open(f, fileName);
    if (err.code != 200)
    {
        return err;
    }
    return Qiniu_File_Stat(*f, &fi);
}

static const Qiniu_Int64 Min_Part_Size = (1 << 20); // 1MB
static const Qiniu_Int64 Max_Part_Size = (1 << 30); // 1GB

Qiniu_Error verifyParam(Qiniu_Client *client, const char *accessKey, const char *bucketName, Qiniu_Multipart_PutExtra *param)
{
    Qiniu_Error err = Qiniu_OK;

    if (param->partSize == 0)
    {
        param->partSize = (4 << 20); // 4M
    }
    if ((param->partSize < Min_Part_Size) || (param->partSize > Max_Part_Size)) // valid part size: 1MB~1GB
    {
        err.code = 400;
        err.message = "partSize must between 1MB and 1GB";
        return err;
    }
    if (param->tryTimes == 0)
    {
        param->tryTimes = 3;
    }
    if (param->upHost == NULL && (param->upHosts == NULL || param->upHostsCount == 0))
    {
        err = _Qiniu_Region_Get_Up_Hosts(client, accessKey, bucketName, &param->upHosts, &param->upHostsCount);
    }

    return err;
}

char *encodeKey(const char *key, bool hasKey)
{
    if (!hasKey)
    {
        char *ret = (char *)malloc(2);
        ret[0] = '~'; // 服务端分片上传协议规定,~表示不设置文件名
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

Qiniu_Error readMedium(struct Qiniu_Record_Medium *medium, char **uploadId, Qiniu_Uint64 *expireAt, Qiniu_UploadPartResp *ret)
{
    Qiniu_Error err = Qiniu_OK;
#define BUFFER_SIZE (1 << 12)
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
    ret->md5 = Qiniu_String_Dup(Qiniu_Json_GetString(blockInfo, "md5", NULL));
    ret->etag = Qiniu_String_Dup(Qiniu_Json_GetString(blockInfo, "etag", NULL));
    if (uploadId != NULL)
    {
        *uploadId = Qiniu_String_Dup(Qiniu_Json_GetString(blockInfo, "uploadId", NULL));
    }
    if (expireAt != NULL)
    {
        *expireAt = Qiniu_Json_GetUInt64(blockInfo, "expireAt", 0);
    }
    cJSON_Delete(blockInfo);
    return err;
#undef BUFFER_SIZE
}

Qiniu_Error writeMedium(struct Qiniu_Record_Medium *medium, const Qiniu_InitPart_Ret *initPartsRet, const Qiniu_UploadPartResp *uploadPartRet)
{
    Qiniu_Error err;
    cJSON *blockInfo = cJSON_CreateObject();
    cJSON_AddItemToObject(blockInfo, "partNum", cJSON_CreateNumber(uploadPartRet->partNum));
    cJSON_AddItemToObject(blockInfo, "md5", cJSON_CreateString(uploadPartRet->md5));
    cJSON_AddItemToObject(blockInfo, "etag", cJSON_CreateString(uploadPartRet->etag));
    cJSON_AddItemToObject(blockInfo, "uploadId", cJSON_CreateString(initPartsRet->uploadId));
    if (initPartsRet->expireAt > 0)
    {
        cJSON_AddItemToObject(blockInfo, "expireAt", cJSON_CreateNumber(initPartsRet->expireAt));
    }
    char *blockInfoJson = cJSON_PrintUnformatted(blockInfo);
    cJSON_Delete(blockInfo);
    if (blockInfoJson == NULL)
    {
        err.code = 400;
        err.message = "cJSON_PrintUnformatted() error";
        return err;
    }
    err = medium->writeEntry(medium, blockInfoJson, NULL);
    free(blockInfoJson);
    if (err.code != 200)
    {
        return err;
    }

    return Qiniu_OK;
}

Qiniu_Error initializeRecorder(Qiniu_Multipart_PutExtra *param,
                               const char *uptoken, const char *key, const char *fileName, Qiniu_FileInfo *fi,
                               Qiniu_Record_Medium *medium, Qiniu_Multipart_Recorder *recorder)
{

    Qiniu_Bool ok = Qiniu_False;
    Qiniu_Error err = Qiniu_OK;
    if (param->recorder != NULL)
    {
        err = Qiniu_Utils_Generate_RecorderKey(uptoken, "v2", key, fileName, &recorder->recorderKey);
        if (err.code != 200)
        {
            return err;
        }
        err = Qiniu_Utils_Find_Medium(param->recorder, recorder->recorderKey, 1, medium, fi, &ok);
        if (err.code != 200)
        {
            return err;
        }
        if (ok)
        {
            recorder->recorderMedium = medium;
            recorder->toLoadProgresses = Qiniu_True;
        }
        else
        {
            err = Qiniu_Utils_New_Medium(param->recorder, recorder->recorderKey, 1, medium, fi);
            if (err.code != 200)
            {
                return err;
            }
            recorder->recorderMedium = medium;
            recorder->toLoadProgresses = Qiniu_False;
        }
    }
    return Qiniu_OK;
}

Qiniu_Error reinitializeRecorder(Qiniu_Multipart_PutExtra *param, Qiniu_FileInfo *fi, Qiniu_Multipart_Recorder *recorder)
{
    Qiniu_Error err;
    recorder->recorderMedium->close(recorder->recorderMedium);
    err = Qiniu_Utils_New_Medium(param->recorder, recorder->recorderKey, 1, recorder->recorderMedium, fi);
    if (err.code != 200)
    {
        return err;
    }
    recorder->toLoadProgresses = Qiniu_False;
    return Qiniu_OK;
}

Qiniu_Error loadProgresses(Qiniu_Multipart_Recorder *recorder, Qiniu_InitPart_Ret *initPartRet, Qiniu_UploadParts_Ret *parts, Qiniu_Multipart_PutExtra *param, Qiniu_FileInfo *fi)
{
    Qiniu_Bool hasNext;
    int blkIdx;
    char *uploadId = NULL;
    Qiniu_Uint64 expireAt = 0;
    const Qiniu_Uint64 now = time(NULL);
    Qiniu_UploadPartResp uploadPartRet;
    if (recorder != NULL && recorder->recorderMedium != NULL && recorder->toLoadProgresses)
    {
        for (;;)
        {
            Qiniu_Error err = recorder->recorderMedium->hasNextEntry(recorder->recorderMedium, &hasNext);
            if (err.code != 200)
            {
                return err;
            }
            if (hasNext)
            {
                if (uploadId != NULL)
                {
                    err = readMedium(recorder->recorderMedium, NULL, NULL, &uploadPartRet);
                }
                else
                {
                    err = readMedium(recorder->recorderMedium, &uploadId, &expireAt, &uploadPartRet);
                }
                if (err.code != 200)
                {
                    return err;
                }
                if (expireAt >= now)
                {
                    *(parts->PartsRet + uploadPartRet.partNum - 1) = uploadPartRet;
                }
                else if (fi != NULL)
                {
                    return reinitializeRecorder(param, fi, recorder);
                }
            }
            else
            {
                break;
            }
        }
    }

    if (initPartRet != NULL && expireAt >= now)
    {
        initPartRet->uploadId = uploadId;
        initPartRet->expireAt = expireAt;
    }

    return Qiniu_OK;
}
