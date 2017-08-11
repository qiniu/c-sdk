/*
 ============================================================================
 Name        : rs.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */
#include "rs.h"
#include "../cJSON/cJSON.h"
#include <time.h>
#include <stdio.h>

/*============================================================================*/
/* type Qiniu_RS_PutPolicy/GetPolicy */

char *Qiniu_RS_PutPolicy_Token(Qiniu_RS_PutPolicy *auth, Qiniu_Mac *mac) {
    int expires;
    time_t deadline;
    char *authstr;
    char *token;

    cJSON *root = cJSON_CreateObject();

    if (auth->scope) {
        cJSON_AddStringToObject(root, "scope", auth->scope);
    }
    if (auth->isPrefixalScope) {
        cJSON_AddNumberToObject(root, "isPrefixalScope", auth->isPrefixalScope);
    }
    if (auth->saveKey) {
        cJSON_AddStringToObject(root, "saveKey", auth->saveKey);
    }
    if (auth->callbackUrl) {
        cJSON_AddStringToObject(root, "callbackUrl", auth->callbackUrl);
    }
    if (auth->callbackHost) {
        cJSON_AddStringToObject(root, "callbackHost", auth->callbackHost);
    }
    if (auth->callbackBody) {
        cJSON_AddStringToObject(root, "callbackBody", auth->callbackBody);
    }
    if (auth->callbackBodyType) {
        cJSON_AddStringToObject(root, "callbackBodyType", auth->callbackBodyType);
    }
    if (auth->callbackFetchKey) {
        cJSON_AddStringToObject(root, "callbackFetchKey", auth->callbackFetchKey);
    }
    if (auth->returnUrl) {
        cJSON_AddStringToObject(root, "returnUrl", auth->returnUrl);
    }
    if (auth->returnBody) {
        cJSON_AddStringToObject(root, "returnBody", auth->returnBody);
    }
    if (auth->endUser) {
        cJSON_AddStringToObject(root, "endUser", auth->endUser);
    }
    if (auth->persistentOps) {
        cJSON_AddStringToObject(root, "persistentOps", auth->persistentOps);
    }
    if (auth->persistentNotifyUrl) {
        cJSON_AddStringToObject(root, "persistentNotifyUrl", auth->persistentNotifyUrl);
    }
    if (auth->persistentPipeline) {
        cJSON_AddStringToObject(root, "persistentPipeline", auth->persistentPipeline);
    }
    if (auth->mimeLimit) {
        cJSON_AddStringToObject(root, "mimeLimit", auth->mimeLimit);
    }
    if (auth->fsizeMin) {
        cJSON_AddNumberToObject(root, "fsizeMin", auth->fsizeLimit);
    }
    if (auth->fsizeLimit) {
        cJSON_AddNumberToObject(root, "fsizeLimit", auth->fsizeLimit);
    }
    if (auth->detectMime) {
        cJSON_AddNumberToObject(root, "detectMime", auth->detectMime);
    }
    if (auth->insertOnly) {
        cJSON_AddNumberToObject(root, "insertOnly", auth->insertOnly);
    }
    if (auth->deleteAfterDays) {
        cJSON_AddNumberToObject(root, "deleteAfterDays", auth->deleteAfterDays);
    }
    if (auth->fileType) {
        cJSON_AddNumberToObject(root, "fileType", auth->fileType);
    }

    if (auth->expires) {
        expires = auth->expires;
    } else {
        expires = 3600; // 1小时
    }
    time(&deadline);
    deadline += expires;
    cJSON_AddNumberToObject(root, "deadline", deadline);

    authstr = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    token = Qiniu_Mac_SignToken(mac, authstr);
    Qiniu_Free(authstr);

    return token;
}

char *Qiniu_RS_GetPolicy_MakeRequest(Qiniu_RS_GetPolicy *auth, const char *baseUrl, Qiniu_Mac *mac) {
    int expires;
    time_t deadline;
    char e[11];
    char *authstr;
    char *token;
    char *request;

    if (auth->expires) {
        expires = auth->expires;
    } else {
        expires = 3600; // 1小时
    }
    time(&deadline);
    deadline += expires;
    sprintf(e, "%u", (unsigned int) deadline);

    if (strchr(baseUrl, '?') != NULL) {
        authstr = Qiniu_String_Concat3(baseUrl, "&e=", e);
    } else {
        authstr = Qiniu_String_Concat3(baseUrl, "?e=", e);
    }

    token = Qiniu_Mac_Sign(mac, authstr);

    request = Qiniu_String_Concat3(authstr, "&token=", token);

    Qiniu_Free(token);
    Qiniu_Free(authstr);

    return request;
}

char *Qiniu_RS_MakeBaseUrl(const char *domain, const char *key) {
    Qiniu_Bool fesc;
    char *baseUrl;
    char *escapedKey = Qiniu_PathEscape(key, &fesc);

    baseUrl = Qiniu_String_Concat(domain, "/", escapedKey, NULL);

    if (fesc) {
        Qiniu_Free(escapedKey);
    }

    return baseUrl;
}

/*============================================================================*/
/* func Qiniu_RS_Stat */

Qiniu_Error Qiniu_RS_Stat(
        Qiniu_Client *self, Qiniu_RS_StatRet *ret, const char *bucket, const char *key) {
    Qiniu_Error err;
    cJSON *root;

    char *entryURI = Qiniu_String_Concat3(bucket, ":", key);
    char *entryURIEncoded = Qiniu_String_Encode(entryURI);
    char *url = Qiniu_String_Concat3(QINIU_RS_HOST, "/stat/", entryURIEncoded);

    Qiniu_Free(entryURI);
    Qiniu_Free(entryURIEncoded);

    err = Qiniu_Client_Call(self, &root, url);
    Qiniu_Free(url);

    if (err.code == 200) {
        ret->hash = Qiniu_Json_GetString(root, "hash", 0);
        ret->mimeType = Qiniu_Json_GetString(root, "mimeType", 0);
        ret->fsize = Qiniu_Json_GetInt64(root, "fsize", 0);
        ret->putTime = Qiniu_Json_GetInt64(root, "putTime", 0);
        ret->type = Qiniu_Json_GetInt64(root, "type", 0);
    }

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_Delete */

Qiniu_Error Qiniu_RS_Delete(Qiniu_Client *self, const char *bucket, const char *key) {
    Qiniu_Error err;

    char *entryURI = Qiniu_String_Concat3(bucket, ":", key);
    char *entryURIEncoded = Qiniu_String_Encode(entryURI);
    char *url = Qiniu_String_Concat3(QINIU_RS_HOST, "/delete/", entryURIEncoded);

    Qiniu_Free(entryURI);
    Qiniu_Free(entryURIEncoded);

    err = Qiniu_Client_CallNoRet(self, url);
    Qiniu_Free(url);

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_Copy */

Qiniu_Error Qiniu_RS_Copy(Qiniu_Client *self, const char *srcBucket, const char *srcKey,
                          const char *destBucket, const char *destKey, Qiniu_Bool force) {
    Qiniu_Error err;

    char *entryURISrc = Qiniu_String_Concat3(srcBucket, ":", srcKey);
    char *entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
    char *entryURIDest = Qiniu_String_Concat3(destBucket, ":", destKey);
    char *entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);
    char *urlPart = Qiniu_String_Concat3(entryURISrcEncoded, "/", entryURIDestEncoded);
    char *forcePart = NULL;
    if (force == Qiniu_True) {
        forcePart = "/force/1";
    } else {
        forcePart = "/force/0";
    }
    char *url = Qiniu_String_Concat(QINIU_RS_HOST, "/copy/", urlPart, forcePart, NULL);

    free(entryURISrc);
    free(entryURISrcEncoded);
    free(entryURIDest);
    free(entryURIDestEncoded);
    free(urlPart);

    err = Qiniu_Client_CallNoRet(self, url);
    free(url);

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_Move */

Qiniu_Error Qiniu_RS_Move(Qiniu_Client *self, const char *srcBucket, const char *srcKey,
                          const char *destBucket, const char *destKey, Qiniu_Bool force) {
    Qiniu_Error err;

    char *entryURISrc = Qiniu_String_Concat3(srcBucket, ":", srcKey);
    char *entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
    char *entryURIDest = Qiniu_String_Concat3(destBucket, ":", destKey);
    char *entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);
    char *urlPart = Qiniu_String_Concat3(entryURISrcEncoded, "/", entryURIDestEncoded);
    char *forcePart = NULL;
    if (force == Qiniu_True) {
        forcePart = "/force/1";
    } else {
        forcePart = "/force/0";
    }
    char *url = Qiniu_String_Concat(QINIU_RS_HOST, "/move/", urlPart, forcePart, NULL);

    free(entryURISrc);
    free(entryURISrcEncoded);
    free(entryURIDest);
    free(entryURIDestEncoded);
    free(urlPart);

    err = Qiniu_Client_CallNoRet(self, url);
    free(url);

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_ChangeMime */

Qiniu_Error Qiniu_RS_ChangeMime(Qiniu_Client *self, const char *bucket, const char *key, const char *newMime) {
    Qiniu_Error err;

    char *entryURI = Qiniu_String_Concat3(bucket, ":", key);
    char *entryURIEncoded = Qiniu_String_Encode(entryURI);
    char *encodedMime = Qiniu_String_Encode(newMime);
    char *url = Qiniu_String_Concat(QINIU_RS_HOST, "/chgm/", entryURIEncoded, "/mime/", encodedMime, NULL);

    Qiniu_Free(entryURI);
    Qiniu_Free(entryURIEncoded);
    Qiniu_Free(encodedMime);

    err = Qiniu_Client_CallNoRet(self, url);
    Qiniu_Free(url);
    return err;
}

/*============================================================================*/
/* func Qiniu_RS_ChangeType */

Qiniu_Error Qiniu_RS_ChangeType(Qiniu_Client *self, const char *bucket, const char *key, const int fileType) {
    Qiniu_Error err;

    char *fileTypeStr = "0";
    if (fileType == 1) {
        fileTypeStr = "1";
    }

    char *entryURI = Qiniu_String_Concat3(bucket, ":", key);
    char *entryURIEncoded = Qiniu_String_Encode(entryURI);
    char *url = Qiniu_String_Concat(QINIU_RS_HOST, "/chtype/", entryURIEncoded, "/type/", fileTypeStr, NULL);

    Qiniu_Free(entryURI);
    Qiniu_Free(entryURIEncoded);

    err = Qiniu_Client_CallNoRet(self, url);
    Qiniu_Free(url);
    return err;
}

/*============================================================================*/
/* func Qiniu_RS_DeleteAfterDays */

Qiniu_Error Qiniu_RS_DeleteAfterDays(Qiniu_Client *self, const char *bucket, const char *key, const int days) {
    Qiniu_Error err;

    char *entryURI = Qiniu_String_Concat3(bucket, ":", key);
    char *entryURIEncoded = Qiniu_String_Encode(entryURI);

    char *daysStr = (char *) malloc(sizeof(int) + 1);
    sprintf(daysStr, "%d", days);
    char *url = Qiniu_String_Concat(QINIU_RS_HOST, "/deleteAfterDays/", entryURIEncoded, "/", daysStr, NULL);

    Qiniu_Free(daysStr);
    Qiniu_Free(entryURI);
    Qiniu_Free(entryURIEncoded);

    err = Qiniu_Client_CallNoRet(self, url);
    Qiniu_Free(url);
    return err;
}

/*============================================================================*/
/* func Qiniu_RS_Fetch */
Qiniu_Error Qiniu_RS_Fetch(Qiniu_Client *self, Qiniu_RS_FetchRet *ret, const char *resURL, const char *bucket,
                           const char *key) {
    Qiniu_Error err;
    cJSON *root;

    char *entryURI = 0;
    if (key) {
        entryURI = Qiniu_String_Concat3(bucket, ":", key);
    } else {
        entryURI = strdup(bucket);
    }

    char *entryURIEncoded = Qiniu_String_Encode(entryURI);
    char *encodedResURL = Qiniu_String_Encode(resURL);

    char *url = Qiniu_String_Concat(QINIU_IOVIP_HOST, "/fetch/", encodedResURL, "/to/", entryURIEncoded, NULL);
    Qiniu_Free(encodedResURL);

    Qiniu_Free(entryURI);
    Qiniu_Free(entryURIEncoded);

    err = Qiniu_Client_Call(self, &root, url);
    Qiniu_Free(url);

    if (err.code == 200) {
        ret->key = Qiniu_Json_GetString(root, "key", 0);
        ret->hash = Qiniu_Json_GetString(root, "hash", 0);
        ret->mimeType = Qiniu_Json_GetString(root, "mimeType", 0);
        ret->fsize = Qiniu_Json_GetInt64(root, "fsize", 0);
    }

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_Prefetch */
Qiniu_Error Qiniu_RS_Prefetch(Qiniu_Client *self, const char *bucket, const char *key) {
    Qiniu_Error err;

    char *entryURI = Qiniu_String_Concat3(bucket, ":", key);
    char *entryURIEncoded = Qiniu_String_Encode(entryURI);
    char *url = Qiniu_String_Concat3(QINIU_IOVIP_HOST, "/prefetch/", entryURIEncoded);

    Qiniu_Free(entryURI);
    Qiniu_Free(entryURIEncoded);

    err = Qiniu_Client_CallNoRet(self, url);
    Qiniu_Free(url);

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_BatchStat */

Qiniu_Error Qiniu_RS_BatchStat(
        Qiniu_Client *self, Qiniu_RS_BatchStatRet *rets,
        Qiniu_RS_EntryPath *entries, Qiniu_ItemCount entryCount) {
    int code;
    Qiniu_Error err;
    cJSON *root, *arrayItem, *dataItem;
    char *body = NULL, *bodyTmp = NULL;
    char *entryURI, *entryURIEncoded, *opBody;
    Qiniu_RS_EntryPath *entry = entries;
    Qiniu_ItemCount curr = 0;
    Qiniu_ItemCount retSize = 0;
    char *url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

    while (curr < entryCount) {
        entryURI = Qiniu_String_Concat3(entry->bucket, ":", entry->key);
        entryURIEncoded = Qiniu_String_Encode(entryURI);
        opBody = Qiniu_String_Concat2("op=/stat/", entryURIEncoded);
        free(entryURI);
        free(entryURIEncoded);

        if (!body) {
            bodyTmp = opBody;
        } else {
            bodyTmp = Qiniu_String_Concat3(body, "&", opBody);
            free(opBody);
        }
        free(body);
        body = bodyTmp;
        curr++;
        entry = &entries[curr];
    }

    err = Qiniu_Client_CallWithBuffer(self, &root,
                                      url, body, strlen(body), "application/x-www-form-urlencoded");
    free(url);
    free(body);

    retSize = cJSON_GetArraySize(root);

    curr = 0;
    while (curr < retSize) {
        arrayItem = cJSON_GetArrayItem(root, curr);
        code = (int) Qiniu_Json_GetInt64(arrayItem, "code", 0);
        dataItem = cJSON_GetObjectItem(arrayItem, "data");

        rets[curr].code = code;

        if (code != 200) {
            rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
        } else {
            rets[curr].data.hash = Qiniu_Json_GetString(dataItem, "hash", 0);
            rets[curr].data.mimeType = Qiniu_Json_GetString(dataItem, "mimeType", 0);
            rets[curr].data.fsize = Qiniu_Json_GetInt64(dataItem, "fsize", 0);
            rets[curr].data.putTime = Qiniu_Json_GetInt64(dataItem, "putTime", 0);
            rets[curr].data.type = Qiniu_Json_GetInt64(dataItem, "type", 0);
        }
        curr++;
    }

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_BatchDelete */

Qiniu_Error Qiniu_RS_BatchDelete(
        Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
        Qiniu_RS_EntryPath *entries, Qiniu_ItemCount entryCount) {
    int code;
    Qiniu_Error err;
    cJSON *root, *arrayItem, *dataItem;
    char *body = NULL, *bodyTmp = NULL;
    char *entryURI, *entryURIEncoded, *opBody;
    Qiniu_ItemCount curr = 0;
    Qiniu_ItemCount retSize = 0;
    Qiniu_RS_EntryPath *entry = entries;
    char *url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

    curr = 0;
    while (curr < entryCount) {
        entryURI = Qiniu_String_Concat3(entry->bucket, ":", entry->key);
        entryURIEncoded = Qiniu_String_Encode(entryURI);
        opBody = Qiniu_String_Concat2("op=/delete/", entryURIEncoded);
        free(entryURI);
        free(entryURIEncoded);

        if (!body) {
            bodyTmp = opBody;
        } else {
            bodyTmp = Qiniu_String_Concat3(body, "&", opBody);
            free(opBody);
        }
        free(body);
        body = bodyTmp;
        curr++;
        entry = &entries[curr];
    }

    err = Qiniu_Client_CallWithBuffer(self, &root,
                                      url, body, strlen(body), "application/x-www-form-urlencoded");
    free(url);
    free(body);

    retSize = cJSON_GetArraySize(root);

    curr = 0;
    while (curr < retSize) {
        arrayItem = cJSON_GetArrayItem(root, curr);
        code = (int) Qiniu_Json_GetInt64(arrayItem, "code", 0);
        dataItem = cJSON_GetObjectItem(arrayItem, "data");

        rets[curr].code = code;

        if (code != 200) {
            rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
        }
        curr++;
    }

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_BatchMove */

Qiniu_Error Qiniu_RS_BatchMove(
        Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
        Qiniu_RS_EntryPathPair *entryPairs, Qiniu_ItemCount entryCount) {
    int code;
    Qiniu_Error err;
    cJSON *root, *arrayItem, *dataItem;
    char *body = NULL, *bodyTmp = NULL;
    char *entryURISrc, *entryURISrcEncoded, *opBody;
    char *entryURIDest, *entryURIDestEncoded, *bodyPart;
    Qiniu_ItemCount curr = 0;
    Qiniu_ItemCount retSize = 0;
    Qiniu_RS_EntryPathPair *entryPair = entryPairs;
    char *url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

    curr = 0;
    while (curr < entryCount) {
        entryURISrc = Qiniu_String_Concat3(entryPair->src.bucket, ":", entryPair->src.key);
        entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
        entryURIDest = Qiniu_String_Concat3(entryPair->dest.bucket, ":", entryPair->dest.key);
        entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);

        char *forceStr = "0";
        if (entryPair->force == Qiniu_True) {
            forceStr = "1";
        }

        bodyPart = Qiniu_String_Concat(entryURISrcEncoded, "/", entryURIDestEncoded, "/force/", forceStr, NULL);
        opBody = Qiniu_String_Concat2("op=/move/", bodyPart);
        free(entryURISrc);
        free(entryURISrcEncoded);
        free(entryURIDest);
        free(entryURIDestEncoded);
        free(bodyPart);

        if (!body) {
            bodyTmp = opBody;
        } else {
            bodyTmp = Qiniu_String_Concat3(body, "&", opBody);
            free(opBody);
        }
        free(body);
        body = bodyTmp;
        curr++;
        entryPair = &entryPairs[curr];
    }

    err = Qiniu_Client_CallWithBuffer(self, &root,
                                      url, body, strlen(body), "application/x-www-form-urlencoded");
    free(url);
    free(body);

    retSize = cJSON_GetArraySize(root);

    curr = 0;
    while (curr < retSize) {
        arrayItem = cJSON_GetArrayItem(root, curr);
        code = (int) Qiniu_Json_GetInt64(arrayItem, "code", 0);
        dataItem = cJSON_GetObjectItem(arrayItem, "data");

        rets[curr].code = code;

        if (code != 200) {
            rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
        }
        curr++;
    }

    return err;
}

/*============================================================================*/
/* func Qiniu_RS_BatchCopy */

Qiniu_Error Qiniu_RS_BatchCopy(
        Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
        Qiniu_RS_EntryPathPair *entryPairs, Qiniu_ItemCount entryCount) {
    int code;
    Qiniu_Error err;
    cJSON *root, *arrayItem, *dataItem;
    char *body = NULL, *bodyTmp = NULL;
    char *entryURISrc, *entryURISrcEncoded, *opBody;
    char *entryURIDest, *entryURIDestEncoded, *bodyPart;
    Qiniu_ItemCount curr = 0;
    Qiniu_ItemCount retSize = 0;
    Qiniu_RS_EntryPathPair *entryPair = entryPairs;
    char *url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

    curr = 0;
    while (curr < entryCount) {
        entryURISrc = Qiniu_String_Concat3(entryPair->src.bucket, ":", entryPair->src.key);
        entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
        entryURIDest = Qiniu_String_Concat3(entryPair->dest.bucket, ":", entryPair->dest.key);
        entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);

        char *forceStr = "0";
        if (entryPair->force == Qiniu_True) {
            forceStr = "1";
        }

        bodyPart = Qiniu_String_Concat(entryURISrcEncoded, "/", entryURIDestEncoded, "/force/", forceStr, NULL);
        opBody = Qiniu_String_Concat2("op=/copy/", bodyPart);
        free(entryURISrc);
        free(entryURISrcEncoded);
        free(entryURIDest);
        free(entryURIDestEncoded);
        free(bodyPart);

        if (!body) {
            bodyTmp = opBody;
        } else {
            bodyTmp = Qiniu_String_Concat3(body, "&", opBody);
            free(opBody);
        }
        free(body);
        body = bodyTmp;
        curr++;
        entryPair = &entryPairs[curr];
    }

    err = Qiniu_Client_CallWithBuffer(self, &root,
                                      url, body, strlen(body), "application/x-www-form-urlencoded");
    free(url);
    free(body);

    retSize = cJSON_GetArraySize(root);

    curr = 0;
    while (curr < retSize) {
        arrayItem = cJSON_GetArrayItem(root, curr);
        code = (int) Qiniu_Json_GetInt64(arrayItem, "code", 0);
        dataItem = cJSON_GetObjectItem(arrayItem, "data");

        rets[curr].code = code;

        if (code != 200) {
            rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
        }
        curr++;
    }

    return err;
}

Qiniu_Error Qiniu_RS_BatchChangeType(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                     Qiniu_RS_EntryChangeType *entries, Qiniu_ItemCount entryCount) {
    int code;
    Qiniu_Error err;
    cJSON *root, *arrayItem, *dataItem;
    char *body = NULL, *bodyTmp = NULL;
    char *entryURI, *entryURIEncoded, *opBody, *bodyPart;

    Qiniu_ItemCount curr = 0;
    Qiniu_ItemCount retSize = 0;
    Qiniu_RS_EntryChangeType *entry = entries;
    char *url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

    curr = 0;
    while (curr < entryCount) {
        entryURI = Qiniu_String_Concat3(entry->bucket, ":", entry->key);
        entryURIEncoded = Qiniu_String_Encode(entryURI);

        char *entryType = "0";
        if (entry->fileType == 1) {
            entryType = "1";
        }

        bodyPart = Qiniu_String_Concat(entryURIEncoded, "/type/", entryType, NULL);
        opBody = Qiniu_String_Concat2("op=/chtype/", bodyPart);
        Qiniu_Free(entryURI);
        Qiniu_Free(entryURIEncoded);
        Qiniu_Free(bodyPart);

        if (!body) {
            bodyTmp = opBody;
        } else {
            bodyTmp = Qiniu_String_Concat3(body, "&", opBody);
            free(opBody);
        }
        free(body);
        body = bodyTmp;
        curr++;
        entry = &entries[curr];
    }

    err = Qiniu_Client_CallWithBuffer(self, &root,
                                      url, body, strlen(body), "application/x-www-form-urlencoded");
    free(url);
    free(body);

    retSize = cJSON_GetArraySize(root);

    curr = 0;
    while (curr < retSize) {
        arrayItem = cJSON_GetArrayItem(root, curr);
        code = (int) Qiniu_Json_GetInt64(arrayItem, "code", 0);
        dataItem = cJSON_GetObjectItem(arrayItem, "data");

        rets[curr].code = code;

        if (code != 200) {
            rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
        }
        curr++;
    }

    return err;
}

Qiniu_Error Qiniu_RS_BatchChangeMime(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                     Qiniu_RS_EntryChangeMime *entries, Qiniu_ItemCount entryCount) {
    int code;
    Qiniu_Error err;
    cJSON *root, *arrayItem, *dataItem;
    char *body = NULL, *bodyTmp = NULL;
    char *entryURI, *entryURIEncoded, *mimeEncoded, *opBody, *bodyPart;

    Qiniu_ItemCount curr = 0;
    Qiniu_ItemCount retSize = 0;
    Qiniu_RS_EntryChangeMime *entry = entries;
    char *url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

    curr = 0;
    while (curr < entryCount) {
        entryURI = Qiniu_String_Concat3(entry->bucket, ":", entry->key);
        entryURIEncoded = Qiniu_String_Encode(entryURI);
        mimeEncoded = Qiniu_String_Encode(entry->mime);

        bodyPart = Qiniu_String_Concat(entryURIEncoded, "/mime/", mimeEncoded, NULL);
        opBody = Qiniu_String_Concat2("op=/chgm/", bodyPart);
        Qiniu_Free(entryURI);
        Qiniu_Free(entryURIEncoded);
        Qiniu_Free(mimeEncoded);
        Qiniu_Free(bodyPart);

        if (!body) {
            bodyTmp = opBody;
        } else {
            bodyTmp = Qiniu_String_Concat3(body, "&", opBody);
            free(opBody);
        }
        free(body);
        body = bodyTmp;
        curr++;
        entry = &entries[curr];
    }

    err = Qiniu_Client_CallWithBuffer(self, &root,
                                      url, body, strlen(body), "application/x-www-form-urlencoded");
    free(url);
    free(body);

    retSize = cJSON_GetArraySize(root);

    curr = 0;
    while (curr < retSize) {
        arrayItem = cJSON_GetArrayItem(root, curr);
        code = (int) Qiniu_Json_GetInt64(arrayItem, "code", 0);
        dataItem = cJSON_GetObjectItem(arrayItem, "data");

        rets[curr].code = code;

        if (code != 200) {
            rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
        }
        curr++;
    }

    return err;
}

Qiniu_Error Qiniu_RS_BatchDeleteAfterDays(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                          Qiniu_RS_EntryDeleteAfterDays *entries, Qiniu_ItemCount entryCount) {
    int code;
    Qiniu_Error err;
    cJSON *root, *arrayItem, *dataItem;
    char *body = NULL, *bodyTmp = NULL;
    char *entryURI, *entryURIEncoded, *daysStr, *opBody, *bodyPart;

    Qiniu_ItemCount curr = 0;
    Qiniu_ItemCount retSize = 0;
    Qiniu_RS_EntryDeleteAfterDays *entry = entries;
    char *url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

    curr = 0;
    while (curr < entryCount) {
        entryURI = Qiniu_String_Concat3(entry->bucket, ":", entry->key);
        entryURIEncoded = Qiniu_String_Encode(entryURI);

        size_t daysLen = snprintf(NULL, 0, "%d", entry->days) + 1;
        daysStr = (char *) malloc(sizeof(char) * daysLen);
        snprintf(daysStr, daysLen, "%d", entry->days);

        bodyPart = Qiniu_String_Concat(entryURIEncoded, "/", daysStr, NULL);
        opBody = Qiniu_String_Concat2("op=/deleteAfterDays/", bodyPart);
        Qiniu_Free(entryURI);
        Qiniu_Free(entryURIEncoded);
        Qiniu_Free(daysStr);
        Qiniu_Free(bodyPart);

        if (!body) {
            bodyTmp = opBody;
        } else {
            bodyTmp = Qiniu_String_Concat3(body, "&", opBody);
            free(opBody);
        }
        free(body);
        body = bodyTmp;
        curr++;
        entry = &entries[curr];
    }

    err = Qiniu_Client_CallWithBuffer(self, &root,
                                      url, body, strlen(body), "application/x-www-form-urlencoded");
    free(url);
    free(body);

    retSize = cJSON_GetArraySize(root);

    curr = 0;
    while (curr < retSize) {
        arrayItem = cJSON_GetArrayItem(root, curr);
        code = (int) Qiniu_Json_GetInt64(arrayItem, "code", 0);
        dataItem = cJSON_GetObjectItem(arrayItem, "data");

        rets[curr].code = code;

        if (code != 200) {
            rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
        }
        curr++;
    }

    return err;
}