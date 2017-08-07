/*
 ============================================================================
 Name        : rsf.c
 Author      : Qiniu.com
 Copyright   : 2017(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : list files from qiniu bucket
 ============================================================================
 */

#include "rsf.h"
#include "../cJSON/cJSON.h"

Qiniu_Error Qiniu_RSF_ListFiles(Qiniu_Client *self, Qiniu_RSF_ListRet *ret, const char *bucket, const char *prefix,
                                const char *delimiter, const char *marker, int limit) {
    Qiniu_Error err;
    int i;

    if (limit < 1 || limit > 1000) {
        err.code = 400;
        err.message = "invalid list limit";
        return err;
    }

    cJSON *root;
    char *encodedBucket = 0;
    char *encodedMarker = 0;
    char *encodedPrefix = 0;
    char *encodedDelimiter = 0;

    char *url;

    Qiniu_Bool escapeBucketOk;
    Qiniu_Bool escapePrefixOk;
    Qiniu_Bool escapeDelimiterOk;
    Qiniu_Bool escapeMarkerOk;

    encodedBucket = Qiniu_QueryEscape(bucket, &escapeBucketOk);

    if (prefix) {
        encodedPrefix = Qiniu_QueryEscape(prefix, &escapePrefixOk);
    } else {
        encodedPrefix = "";
    }

    if (marker) {
        encodedMarker = Qiniu_QueryEscape(marker, &escapeMarkerOk);
    } else {
        marker = "";
    }

    if (delimiter) {
        encodedDelimiter = Qiniu_QueryEscape(delimiter, &escapeDelimiterOk);
    } else {
        encodedDelimiter = "";
    }

    char *limitStr = (char *) malloc(sizeof(int) + 1);
    sprintf(limitStr, "%d", limit);

    url = Qiniu_String_Concat(QINIU_RSF_HOST, "/list?", "bucket=", encodedBucket, "&prefix=", encodedPrefix,
                              "&delimiter=", encodedDelimiter, "&marker=", encodedMarker, "&limit=", limitStr, 0);

    Qiniu_Free(limitStr);
    if (escapeBucketOk) {
        Qiniu_Free(encodedBucket);
    }

    if (prefix && escapePrefixOk) {
        Qiniu_Free(encodedPrefix);
    }

    if (delimiter && escapeDelimiterOk) {
        Qiniu_Free(encodedDelimiter);
    }

    if (marker && escapeMarkerOk) {
        Qiniu_Free(encodedMarker);
    }

    err = Qiniu_Client_Call(self, root, url);
    Qiniu_Free(url);
    if (err.code == 200) {

    }

    return err;
}