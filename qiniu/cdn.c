#include <openssl/md5.h>
#include <ctype.h>
#include "base.h"
#include "cdn.h"
#include "tm.h"
#include "../cJSON/cJSON.h"
#include <string.h>

char *Qiniu_CDN_CreateTimestampAntiLeechURL(const char *host, const char *fileName, char *queryStr,
                                            Qiniu_Uint64 deadline, const char *cryptKey) {
    char *finalURL = NULL;
    Qiniu_Bool fileNameEscapeOk = Qiniu_False;
    Qiniu_Bool queryStrEscapeOk = Qiniu_False;
    char *fileNameEscaped = Qiniu_PathEscape(fileName, &fileNameEscapeOk);
    char *queryStrEscaped = NULL;

    char expireHex[20];
    sprintf(expireHex, "%0llx", deadline);

    if (queryStr != NULL && strcmp("", queryStr) != 0) {
        queryStrEscaped = Qiniu_PathEscape(queryStr, &queryStrEscapeOk);
    }

    char *path = Qiniu_String_Concat2("/", fileNameEscaped);
    char *signStr = Qiniu_String_Concat(cryptKey, path, expireHex, NULL);
    char *sign = (char *) Qiniu_MD5_HexStr(signStr);

    if (queryStrEscaped != NULL) {
        finalURL = Qiniu_String_Concat(host, path, "?", queryStrEscaped, "&sign=", sign, "&t=", expireHex, NULL);
    } else {
        finalURL = Qiniu_String_Concat(host, path, "?sign=", sign, "&t=", expireHex, NULL);
    }

    if (fileNameEscapeOk == Qiniu_True) {
        Qiniu_Free(fileNameEscaped);
    }
    if (queryStrEscapeOk == Qiniu_True) {
        Qiniu_Free(queryStrEscaped);
    }
    Qiniu_Free(path);
    Qiniu_Free(signStr);
    Qiniu_Free(sign);
    return finalURL;
}

Qiniu_Error Qiniu_CDN_RefreshUrls(Qiniu_Client *self, Qiniu_CDN_RefreshRet *ret, const char *urls[],
                                  const int urlsCount) {
    Qiniu_Error err;

    char *path = "/v2/tune/refresh";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);
    Qiniu_Json *root = cJSON_CreateObject();
    Qiniu_Json *pUrls = cJSON_CreateStringArray(urls, urlsCount);
    cJSON_AddItemToObject(root, "urls", pUrls);
    char *body = cJSON_PrintUnformatted(root);

    Qiniu_Json *jsonRet = NULL;

    err = Qiniu_Client_CallWithBuffer2(
            self, &jsonRet, url, body, strlen(body), "application/json");

    if (err.code == 200) {
        err = Qiniu_Parse_CDNRefreshRet(jsonRet, ret);
    }

    Qiniu_Free(url);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

Qiniu_Error Qiniu_CDN_RefreshDirs(Qiniu_Client *self, Qiniu_CDN_RefreshRet *ret, const char *dirs[],
                                  const int dirsCount) {
    Qiniu_Error err;

    char *path = "/v2/tune/refresh";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    Qiniu_Json *root = cJSON_CreateObject();
    Qiniu_Json *pDirs = cJSON_CreateStringArray(dirs, dirsCount);
    cJSON_AddItemToObject(root, "dirs", pDirs);
    char *body = cJSON_PrintUnformatted(root);

    Qiniu_Json *jsonRet = NULL;

    err = Qiniu_Client_CallWithBuffer2(
            self, &jsonRet, url, body, strlen(body), "application/json");

    if (err.code == 200) {
        err = Qiniu_Parse_CDNRefreshRet(jsonRet, ret);
    }

    Qiniu_Free(url);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

Qiniu_Error Qiniu_CDN_PrefetchUrls(Qiniu_Client *self, Qiniu_CDN_PrefetchRet *ret, const char *urls[],
                                   const int urlsCount) {
    Qiniu_Error err;

    char *path = "/v2/tune/prefetch";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    Qiniu_Json *root = cJSON_CreateObject();
    Qiniu_Json *url_a = cJSON_CreateStringArray(urls, urlsCount);
    cJSON_AddItemToObject(root, "urls", url_a);
    char *body = cJSON_PrintUnformatted(root);

    Qiniu_Json *jsonRet = NULL;

    err = Qiniu_Client_CallWithBuffer2(
            self, &jsonRet, url, body, strlen(body), "application/json");


    if (err.code == 200) {
        err = Qiniu_Parse_CDNPrefetchRet(jsonRet, ret);
    }

    Qiniu_Free(url);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

extern Qiniu_Error Qiniu_CDN_GetFluxData(
        Qiniu_Client *self,
        Qiniu_CDN_FluxRet *ret,
        const char *startDate,
        const char *endDate,
        const char *granularity,
        char *domains[],
        const int num) {
    Qiniu_Error err;

    char *path = "/v2/tune/flux";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    char *domains_str = Qiniu_String_Join(";", domains, num);

    Qiniu_Json *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "startDate", startDate);
    cJSON_AddStringToObject(root, "endDate", endDate);
    cJSON_AddStringToObject(root, "granularity", granularity);
    cJSON_AddStringToObject(root, "domains", domains_str);
    char *body = cJSON_PrintUnformatted(root);

    Qiniu_Json *jsonRet = NULL;

    err = Qiniu_Client_CallWithBuffer2(
            self, &jsonRet, url, body, strlen(body), "application/json");

    if (err.code == 200) {
        err = Qiniu_Parse_CDNFluxRet(jsonRet, ret, domains, num);
    }

    Qiniu_Free(url);
    Qiniu_Free(domains_str);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

extern Qiniu_Error Qiniu_CDN_GetBandwidthData(
        Qiniu_Client *self,
        Qiniu_CDN_BandwidthRet *ret,
        const char *startDate,
        const char *endDate,
        const char *granularity,
        char *domains[],
        const int num) {
    Qiniu_Error err;

    char *path = "/v2/tune/bandwidth";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    char *domains_str = Qiniu_String_Join(";", domains, num);

    Qiniu_Json *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "startDate", startDate);
    cJSON_AddStringToObject(root, "endDate", endDate);
    cJSON_AddStringToObject(root, "granularity", granularity);
    cJSON_AddStringToObject(root, "domains", domains_str);
    char *body = cJSON_PrintUnformatted(root);

    Qiniu_Json *jsonRet = NULL;

    err = Qiniu_Client_CallWithBuffer2(
            self, &jsonRet, url, body, strlen(body), "application/json");


    if (err.code == 200) {
        err = Qiniu_Parse_CDNBandwidthRet(jsonRet, ret, domains, num);
    }

    Qiniu_Free(url);
    Qiniu_Free(domains_str);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

extern Qiniu_Error Qiniu_CDN_GetLogList(Qiniu_Client *self, Qiniu_CDN_LogListRet *ret,
                                        char *domains[], const int domainsCount, const char *day) {
    Qiniu_Error err;

    char *path = "/v2/tune/log/list";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    char *domainsStr = Qiniu_String_Join(";", domains, domainsCount);

    Qiniu_Json *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "day", day);
    cJSON_AddStringToObject(root, "domains", domainsStr);
    char *body = cJSON_PrintUnformatted(root);

    Qiniu_Json *jsonRet = NULL;

    err = Qiniu_Client_CallWithBuffer2(
            self, &jsonRet, url, body, strlen(body), "application/json");


    if (err.code == 200) {
        err = Qiniu_Parse_CDNLogListRet(jsonRet, ret, domains, domainsCount);
    }

    Qiniu_Free(url);
    Qiniu_Free(domainsStr);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

Qiniu_Error Qiniu_Parse_CDNRefreshRet(Qiniu_Json *root, Qiniu_CDN_RefreshRet *ret) {
    Qiniu_Error err;
    int i;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", NULL);
    ret->requestId = Qiniu_Json_GetString(root, "requestId", NULL);

    Qiniu_Json *invalidUrls = cJSON_GetObjectItem(root, "invalidUrls");
    int invalidUrlsCount = cJSON_GetArraySize(invalidUrls);
    if (invalidUrlsCount > 0) {
        ret->invalidUrlsCount = invalidUrlsCount;
        ret->invalidUrls = (char **) malloc(sizeof(char **) * invalidUrlsCount);
        char **pUrls = ret->invalidUrls;

        for (i = 0; i < invalidUrlsCount; i++) {
            *pUrls = cJSON_GetArrayItem(invalidUrls, i)->valuestring;
            ++pUrls;
        }
    } else {
        ret->invalidUrls = NULL;
    }

    Qiniu_Json *invalidDirs = cJSON_GetObjectItem(root, "invalidDirs");
    int invalidDirsCount = cJSON_GetArraySize(invalidDirs);
    if (invalidDirsCount > 0) {
        ret->invalidDirsCount = invalidDirsCount;
        ret->invalidDirs = (char **) malloc(sizeof(char **) * invalidDirsCount);
        char **pDirs = ret->invalidDirs;

        for (i = 0; i < invalidDirsCount; i++) {
            *pDirs = cJSON_GetArrayItem(invalidDirs, i)->valuestring;
            ++pDirs;
        }
    } else {
        ret->invalidDirs = NULL;
    }

    ret->urlQuotaDay = Qiniu_Json_GetInt(root, "urlQuotaDay", 0);
    ret->urlSurplusDay = Qiniu_Json_GetInt(root, "urlSurplusDay", 0);
    ret->dirQuotaDay = Qiniu_Json_GetInt(root, "dirQuotaDay", 0);
    ret->dirSurplusDay = Qiniu_Json_GetInt(root, "dirSurplusDay", 0);

    err.code = 200;
    err.message = "OK";

    return err;
}

Qiniu_Error Qiniu_Parse_CDNPrefetchRet(Qiniu_Json *root, Qiniu_CDN_PrefetchRet *ret) {
    Qiniu_Error err;
    int i;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", "");
    ret->requestId = Qiniu_Json_GetString(root, "requestId", "");

    Qiniu_Json *invalidUrls = cJSON_GetObjectItem(root, "invalidUrls");
    int invalidUrlsCount = cJSON_GetArraySize(invalidUrls);
    if (invalidUrlsCount > 0) {
        ret->invalidUrlsCount = invalidUrlsCount;
        ret->invalidUrls = (char **) malloc(sizeof(char **) * invalidUrlsCount);
        char **pUrls = ret->invalidUrls;

        for (i = 0; i < invalidUrlsCount; i++) {
            *pUrls = cJSON_GetArrayItem(invalidUrls, i)->valuestring;
            ++pUrls;
        }
    } else {
        ret->invalidUrls = NULL;
    }

    ret->quotaDay = Qiniu_Json_GetInt(root, "quotaDay", 0);
    ret->surplusDay = Qiniu_Json_GetInt(root, "surplusDay", 0);

    err.code = 200;
    err.message = "OK";

    return err;
}

Qiniu_Error Qiniu_Parse_CDNFluxRet(Qiniu_Json *root, Qiniu_CDN_FluxRet *ret, char *domains[],
                                   const int domainsCount) {
    Qiniu_Error err;
    int i, j;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", NULL);
    ret->domainsCount = domainsCount;

    //parse time
    int timeCount = Qiniu_Json_GetArraySize(root, "time", 0);
    if (timeCount == 0) {
        ret->time = NULL;
        ret->timeCount = 0;
        ret->data = NULL;

        err.code = 9999;
        err.message = "No cdn flux time";
        return err;
    }

    Qiniu_Json *time = Qiniu_Json_GetObjectItem(root, "time", NULL);
    ret->timeCount = timeCount;
    ret->time = (char **) malloc(sizeof(char **) * timeCount);
    char **pTime = ret->time;
    for (i = 0; i < timeCount; i++) {
        char *tVal = Qiniu_Json_GetArrayItem(time, i, NULL)->valuestring;
        *pTime = tVal;
        ++pTime;
    }

    //parse data
    Qiniu_Json *data = Qiniu_Json_GetObjectItem(root, "data", NULL);
    if (data == NULL) {
        ret->data = NULL;
        err.code = 9999;
        err.message = "No cdn flux data";
        return err;
    }

    ret->data = (Qiniu_CDN_FluxData *) malloc(sizeof(Qiniu_CDN_FluxData) * domainsCount);
    for (i = 0; i < domainsCount; ++i) {
        char *domain = domains[i];

        Qiniu_CDN_FluxData fluxData;
        fluxData.domain = domain;

        Qiniu_Json *domainFluxObj = Qiniu_Json_GetObjectItem(data, domain, NULL);
        if (domainFluxObj == NULL) {
            fluxData.chinaCount = 0;
            fluxData.china = NULL;
            fluxData.overseaCount = 0;
            fluxData.oversea = NULL;
        } else {
            //parse china
            Qiniu_Json *china = Qiniu_Json_GetObjectItem(domainFluxObj, "china", NULL);
            if (china == NULL) {
                fluxData.chinaCount = 0;
                fluxData.china = NULL;
            } else {
                int chinaCount = Qiniu_Json_GetArraySize(domainFluxObj, "china", 0);
                fluxData.chinaCount = chinaCount;
                fluxData.china = (Qiniu_Uint64 *) malloc(sizeof(Qiniu_Uint64) * chinaCount);
                for (j = 0; j < chinaCount; j++) {
                    fluxData.china[j] = (Qiniu_Uint64) Qiniu_Json_GetArrayItem(china, j, 0)->valuedouble;
                }
            }

            //parse oversea
            Qiniu_Json *oversea = Qiniu_Json_GetObjectItem(domainFluxObj, "oversea", NULL);
            if (oversea == NULL) {
                fluxData.overseaCount = 0;
                fluxData.oversea = NULL;
            } else {
                int overseaCount = Qiniu_Json_GetArraySize(domainFluxObj, "oversea", 0);
                fluxData.overseaCount = overseaCount;
                fluxData.oversea = (Qiniu_Uint64 *) malloc(sizeof(Qiniu_Uint64) * overseaCount);
                for (j = 0; j < overseaCount; j++) {
                    fluxData.oversea[j] = (Qiniu_Uint64) Qiniu_Json_GetArrayItem(oversea, j, 0)->valuedouble;
                }
            }

        }
        ret->data[i] = fluxData;
    }

    err.code = 200;
    err.message = "OK";

    return err;
}

Qiniu_Error Qiniu_Parse_CDNBandwidthRet(Qiniu_Json *root, Qiniu_CDN_BandwidthRet *ret,
                                        char *domains[], const int domainsCount) {
    Qiniu_Error err;
    int i, j;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", NULL);
    ret->domainsCount = domainsCount;

    //parse time
    int timeCount = Qiniu_Json_GetArraySize(root, "time", 0);
    if (timeCount == 0) {
        ret->time = NULL;
        ret->timeCount = 0;
        ret->data = NULL;

        err.code = 9999;
        err.message = "No cdn bandwidth time";
        return err;
    }

    Qiniu_Json *time = Qiniu_Json_GetObjectItem(root, "time", NULL);
    ret->timeCount = timeCount;
    ret->time = (char **) malloc(sizeof(char **) * timeCount);
    char **pTime = ret->time;
    for (i = 0; i < timeCount; i++) {
        char *tVal = Qiniu_Json_GetArrayItem(time, i, NULL)->valuestring;
        *pTime = tVal;
        ++pTime;
    }

    //parse data
    Qiniu_Json *data = Qiniu_Json_GetObjectItem(root, "data", NULL);
    if (data == NULL) {
        ret->data = NULL;
        err.code = 9999;
        err.message = "No cdn bandwidth data";
        return err;
    }

    ret->data = (Qiniu_CDN_BandwidthData *) malloc(sizeof(Qiniu_CDN_BandwidthData) * domainsCount);
    for (i = 0; i < domainsCount; ++i) {
        char *domain = domains[i];

        Qiniu_CDN_BandwidthData bandData;
        bandData.domain = domain;

        Qiniu_Json *domainBandObj = Qiniu_Json_GetObjectItem(data, domain, NULL);
        if (domainBandObj == NULL) {
            bandData.chinaCount = 0;
            bandData.china = NULL;
            bandData.overseaCount = 0;
            bandData.oversea = NULL;
        } else {
            //parse china
            Qiniu_Json *china = Qiniu_Json_GetObjectItem(domainBandObj, "china", NULL);
            if (china == NULL) {
                bandData.chinaCount = 0;
                bandData.china = NULL;
            } else {
                int chinaCount = Qiniu_Json_GetArraySize(domainBandObj, "china", 0);
                bandData.chinaCount = chinaCount;
                bandData.china = (Qiniu_Uint64 *) malloc(sizeof(Qiniu_Uint64) * chinaCount);
                for (j = 0; j < chinaCount; j++) {
                    bandData.china[j] = (Qiniu_Uint64) Qiniu_Json_GetArrayItem(china, j, 0)->valuedouble;
                }
            }

            //parse oversea
            Qiniu_Json *oversea = Qiniu_Json_GetObjectItem(domainBandObj, "oversea", NULL);
            if (oversea == NULL) {
                bandData.overseaCount = 0;
                bandData.oversea = NULL;
            } else {
                int overseaCount = Qiniu_Json_GetArraySize(domainBandObj, "oversea", 0);
                bandData.overseaCount = overseaCount;
                bandData.oversea = (Qiniu_Uint64 *) malloc(sizeof(Qiniu_Uint64) * overseaCount);
                for (j = 0; j < overseaCount; j++) {
                    bandData.oversea[j] = (Qiniu_Uint64) Qiniu_Json_GetArrayItem(oversea, j, 0)->valuedouble;
                }
            }

        }
        ret->data[i] = bandData;
    }

    err.code = 200;
    err.message = "OK";

    return err;
}

Qiniu_Error Qiniu_Parse_CDNLogListRet(Qiniu_Json *root, Qiniu_CDN_LogListRet *ret, char *domains[],
                                      const int domainsCount) {
    Qiniu_Error err;
    int i, j;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", NULL);
    ret->domainsCount = domainsCount;

    Qiniu_Json *data = Qiniu_Json_GetObjectItem(root, "data", NULL);
    if (data == NULL) {
        ret->data = NULL;
        err.code = 9999;
        err.message = "No cdn log data";
        return err;
    }

    //parse log list
    ret->data = (Qiniu_CDN_LogListData *) malloc(sizeof(Qiniu_CDN_LogListData) * domainsCount);
    for (i = 0; i < domainsCount; i++) {
        char *domain = domains[i];

        Qiniu_CDN_LogListData logData;
        logData.domain = domain;

        int itemsCount = Qiniu_Json_GetArraySize(data, domain, 0);
        if (itemsCount == 0) {
            logData.items = NULL;
            logData.itemsCount = 0;
        } else {
            logData.itemsCount = itemsCount;
            Qiniu_Json *domainLogs = Qiniu_Json_GetObjectItem(data, domain, NULL);
            logData.items = (Qiniu_CDN_LogListDataItem *) malloc(sizeof(Qiniu_CDN_LogListDataItem) * itemsCount);
            for (j = 0; j < itemsCount; j++) {
                Qiniu_Json *sub = Qiniu_Json_GetArrayItem(domainLogs, j, NULL);
                Qiniu_CDN_LogListDataItem logItem;
                logItem.name = Qiniu_Json_GetString(sub, "name", NULL);
                logItem.url = Qiniu_Json_GetString(sub, "url", NULL);
                logItem.size = Qiniu_Json_GetInt64(sub, "size", 0);
                logItem.mtime = Qiniu_Json_GetInt64(sub, "mtime", 0);

                logData.items[j] = logItem;
            }
        }
        ret->data[i] = logData;
    }

    err.code = 200;
    err.message = "OK";
    return err;
}

void Qiniu_Free_CDNRefreshRet(Qiniu_CDN_RefreshRet *ret) {
    if (ret->invalidUrls != NULL) {
        Qiniu_Free(ret->invalidUrls);
    }

    if (ret->invalidDirs != NULL) {
        Qiniu_Free(ret->invalidDirs);
    }
}

void Qiniu_Free_CDNPrefetchRet(Qiniu_CDN_PrefetchRet *ret) {
    if (ret->invalidUrls != NULL) {
        Qiniu_Free(ret->invalidUrls);
    }
}

void Qiniu_Free_CDNFluxRet(Qiniu_CDN_FluxRet *ret) {
    int i;

    if (ret->data != NULL) {
        for (i = 0; i < ret->domainsCount; ++i) {
            if (ret->data[i].china != NULL) {
                Qiniu_Free(ret->data[i].china);
            }
            if (ret->data[i].oversea != NULL) {
                Qiniu_Free(ret->data[i].oversea);
            }
        }
        Qiniu_Free(ret->data);
    }
}

void Qiniu_Free_CDNBandwidthRet(Qiniu_CDN_BandwidthRet *ret) {
    int i;

    if (ret->data != NULL) {
        for (i = 0; i < ret->domainsCount; ++i) {
            if (ret->data[i].china != NULL) {
                Qiniu_Free(ret->data[i].china);
            }
            if (ret->data[i].oversea != NULL) {
                Qiniu_Free(ret->data[i].oversea);
            }
        }
        Qiniu_Free(ret->data);
    }
}

void Qiniu_Free_CDNLogListRet(Qiniu_CDN_LogListRet *ret) {
    int i;
    if (ret->data != NULL) {
        for (i = 0; i < ret->domainsCount; i++) {
            if (ret->data[i].itemsCount > 0) {
                Qiniu_Free(ret->data[i].items);
            }
        }
        Qiniu_Free(ret->data);
    }
}