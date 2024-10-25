/*
 ============================================================================
 Name        : fop.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include <curl/curl.h>

#include "fop.h"
#include "private/region.h"
#include "../cJSON/cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void _Qiniu_Parse_Date_Time(char *datetime_string, Qiniu_DateTime *dt);

#ifdef __cplusplus
}
#endif

Qiniu_Error Qiniu_FOP_Pfop_v2(Qiniu_Client *self, Qiniu_FOP_PfopRet *ret, Qiniu_FOP_PfopParams *params)
{
    Qiniu_Error err;
    cJSON *root;
    char *fopsStr = NULL;
    char *encodedBucket = NULL;
    char *encodedKey = NULL;
    char *encodedFops = NULL;
    char *encodedNotifyURL = NULL;
    char *encodedPipeline = NULL;
    char *forceStr = NULL;
    char *typeStr = NULL;
    char *url = NULL;
    char *body = NULL;
    Qiniu_Bool escapeBucketOk = Qiniu_False;
    Qiniu_Bool escapeKeyOk = Qiniu_False;
    Qiniu_Bool escapeFopsOk = Qiniu_False;
    Qiniu_Bool escapePipelineOk = Qiniu_False;
    Qiniu_Bool escapeNotifyURLOk = Qiniu_False;

    // Add encoded bucket
    encodedBucket = Qiniu_QueryEscape(params->bucket, &escapeBucketOk);
    encodedKey = Qiniu_QueryEscape(params->key, &escapeKeyOk);
    if (params->fopCount > 0 || params->fops){
        fopsStr = Qiniu_String_Join(";", params->fops, params->fopCount);
        encodedFops = Qiniu_QueryEscape(fopsStr, &escapeFopsOk);
        Qiniu_Free(fopsStr);
    }

    if (params->pipeline)
    {
        encodedPipeline = Qiniu_QueryEscape(params->pipeline, &escapePipelineOk);
    }
    else
    {
        encodedPipeline = "";
    }
    if (params->notifyURL)
    {
        encodedNotifyURL = Qiniu_QueryEscape(params->notifyURL, &escapeNotifyURLOk);
    }
    else
    {
        encodedNotifyURL = "";
    }
    if (params->force == 1)
    {
        forceStr = "1";
    }
    else
    {
        forceStr = "0";
    }
    if (params->type == 1)
    {
        typeStr = "1";
    }
    else
    {
        typeStr = "0";
    }

    if (escapeFopsOk)
    {
        body = Qiniu_String_Concat("bucket=", encodedBucket, "&key=", encodedKey, "&fops=", encodedFops,
                                   "&pipeline=", encodedPipeline, "&notifyURL=", encodedNotifyURL, "&force=", forceStr,
                                   "&type=", typeStr, NULL);
    }
    else
    {
        body = Qiniu_String_Concat("bucket=", encodedBucket, "&key=", encodedKey, "&workflowTemplateID=", params->workflowTemplateID,
                                   "&pipeline=", encodedPipeline, "&notifyURL=", encodedNotifyURL, "&force=", forceStr,
                                   "&type=", typeStr, NULL);
    }

    if (escapeBucketOk)
    {
        Qiniu_Free(encodedBucket);
    }

    if (escapeKeyOk)
    {
        Qiniu_Free(encodedKey);
    }

    if (escapeFopsOk)
    {
        Qiniu_Free(encodedFops);
    }

    if (params->pipeline && escapePipelineOk)
    {
        Qiniu_Free(encodedPipeline);
    }

    if (params->notifyURL && escapeNotifyURLOk)
    {
        Qiniu_Free(encodedNotifyURL);
    }

    const char *apiHost;
    err = _Qiniu_Region_Get_Api_Host(self, NULL, NULL, &apiHost);
    if (err.code != 200)
    {
        return err;
    }
    url = Qiniu_String_Concat2(apiHost, "/pfop/");
    err = Qiniu_Client_CallWithBuffer(
        self,
        &root,
        url,
        body,
        strlen(body),
        "application/x-www-form-urlencoded");
    Qiniu_Free((void *)body);
    Qiniu_Free((void *)url);
    if (err.code == 200)
    {
        ret->persistentId = Qiniu_Json_GetString(root, "persistentId", 0);
    }

    return err;
}

Qiniu_Error Qiniu_FOP_Pfop(Qiniu_Client *self, Qiniu_FOP_PfopRet *ret, const char *bucket, const char *key,
                           char *fops[], int fopCount, const char *pipeline, const char *notifyURL, int force)
{
    Qiniu_FOP_PfopParams params;
    Qiniu_Zero(params);
    params.bucket = bucket;
    params.key = key;
    params.fops = (char **)fops;
    params.fopCount = fopCount;
    params.pipeline = pipeline;
    params.notifyURL = notifyURL;
    params.force = force;
    return Qiniu_FOP_Pfop_v2(self, ret, &params);
} // Qiniu_FOP_Pfop

Qiniu_Error Qiniu_FOP_Prefop(Qiniu_Client *self, Qiniu_FOP_PrefopRet *ret, Qiniu_FOP_PrefopItemRet *itemsRet, Qiniu_ItemCount *itemsCount, const char *persistentId, Qiniu_ItemCount maxItemsCount)
{
    Qiniu_Error err;
    cJSON *root, *items, *item;
    Qiniu_ItemCount curIndex;
    char *encodedPersistentId = NULL, *url = NULL, *creationDateStr = NULL;
    Qiniu_ReadBuf emptyBodyBuf = {NULL, 0, 0};
    Qiniu_Reader emptyBody = Qiniu_BufReader(&emptyBodyBuf, 0, 0);
    Qiniu_Bool escapePersistentIdOk;

    encodedPersistentId = Qiniu_QueryEscape(persistentId, &escapePersistentIdOk);

    const char *apiHost;
    err = _Qiniu_Region_Get_Api_Host(self, NULL, NULL, &apiHost);
    if (err.code != 200)
    {
        return err;
    }
    url = Qiniu_String_Concat(apiHost, "/status/get/prefop?id=", encodedPersistentId, NULL);
    if (escapePersistentIdOk)
    {
        Qiniu_Free(encodedPersistentId);
    }

    err = Qiniu_Client_CallWithMethod(
        self,
        &root,
        url,
        emptyBody,
        0,
        "application/x-www-form-urlencoded",
        "GET",
        NULL);
    Qiniu_Free(url);
    if (err.code != 200)
    {
        return err;
    }

    ret->id = Qiniu_Json_GetString(root, "id", NULL);
    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->desc = Qiniu_Json_GetString(root, "desc", NULL);
    ret->inputBucket = Qiniu_Json_GetString(root, "inputBucket", NULL);
    ret->inputKey = Qiniu_Json_GetString(root, "inputBucket", NULL);
    ret->taskFrom = Qiniu_Json_GetString(root, "taskFrom", NULL);
    ret->type = Qiniu_Json_GetInt(root, "type", 0);
    creationDateStr = (char *)Qiniu_Json_GetString(root, "creationDate", NULL);
    if (creationDateStr != NULL)
    {
        _Qiniu_Parse_Date_Time(creationDateStr, &ret->creationDate);
    }

    *itemsCount = Qiniu_Json_GetArraySize(root, "items", 0);
    items = Qiniu_Json_GetObjectItem(root, "items", 0);
    for (curIndex = 0; curIndex < *itemsCount && curIndex < maxItemsCount; curIndex++)
    {
        item = cJSON_GetArrayItem(items, curIndex);
        itemsRet[curIndex].cmd = Qiniu_Json_GetString(item, "cmd", NULL);
        itemsRet[curIndex].code = Qiniu_Json_GetInt(item, "code", 0);
        itemsRet[curIndex].desc = Qiniu_Json_GetString(item, "desc", NULL);
        itemsRet[curIndex].error = Qiniu_Json_GetString(item, "error", NULL);
        itemsRet[curIndex].hash = Qiniu_Json_GetString(item, "hash", NULL);
        itemsRet[curIndex].key = Qiniu_Json_GetString(item, "key", NULL);
        itemsRet[curIndex].returnOld = Qiniu_Json_GetInt(item, "returnOld", 0);
    }
    return err;
}
