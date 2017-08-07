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
#include "../cJSON/cJSON.h"

Qiniu_Error Qiniu_FOP_Pfop(Qiniu_Client *self, Qiniu_FOP_PfopRet *ret, const char *bucket, const char *key,
                           char *fops[], int fopCount, const char *pipeline, const char *notifyURL, int force) {
    Qiniu_Error err;
    cJSON *root = 0;
    char *fopsStr = 0;
    char *encodedBucket = 0;
    char *encodedKey = 0;
    char *encodedFops = 0;
    char *encodedNotifyURL = 0;
    char *encodedPipeline = 0;
    char *forceStr = 0;
    char *url = 0;
    char *body = 0;
    Qiniu_Bool escapeBucketOk;
    Qiniu_Bool escapeKeyOk;
    Qiniu_Bool escapeFopsOk;
    Qiniu_Bool escapePipelineOk;
    Qiniu_Bool escapeNotifyURLOk;

    // Add encoded bucket
    encodedBucket = Qiniu_QueryEscape(bucket, &escapeBucketOk);
    encodedKey = Qiniu_QueryEscape(key, &escapeKeyOk);
    fopsStr = Qiniu_String_Join(";", fops, fopCount);
    encodedFops = Qiniu_QueryEscape(fopsStr, &escapeFopsOk);
    Qiniu_Free(fopsStr);

    if (pipeline) {
        encodedPipeline = Qiniu_QueryEscape(pipeline, &escapePipelineOk);
    } else {
        encodedPipeline = "";
    }
    if (notifyURL) {
        encodedNotifyURL = Qiniu_QueryEscape(notifyURL, &escapeNotifyURLOk);
    } else {
        encodedNotifyURL = "";
    }
    if (force == 1) {
        forceStr = "1";
    } else {
        forceStr = "0";
    }

    body = Qiniu_String_Concat("bucket=", encodedBucket, "&key=", encodedKey, "&fops=", encodedFops,
                               "&pipeline=", encodedPipeline, "&notifyURL=", encodedNotifyURL, "&force=", forceStr, 0);
    if (escapeBucketOk) {
        Qiniu_Free(encodedBucket);
    }

    if (escapeKeyOk) {
        Qiniu_Free(encodedKey);
    }

    if (escapeFopsOk) {
        Qiniu_Free(encodedFops);
    }

    if (pipeline && escapePipelineOk) {
        Qiniu_Free(encodedPipeline);
    }

    if (notifyURL && escapeNotifyURLOk) {
        Qiniu_Free(encodedNotifyURL);
    }

    url = Qiniu_String_Concat2(QINIU_API_HOST, "/pfop/");
    err = Qiniu_Client_CallWithBuffer(
            self,
            &root,
            url,
            body,
            strlen(body),
            "application/x-www-form-urlencoded"
    );
    Qiniu_Free(url);
    Qiniu_Free(body);
    if (err.code == 200) {
        ret->persistentId = Qiniu_Json_GetString(root, "persistentId", 0);
    }

    return err;
} // Qiniu_FOP_Pfop
