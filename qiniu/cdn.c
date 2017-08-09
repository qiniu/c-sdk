#include <openssl/md5.h>
#include <ctype.h>
#include "base.h"
#include "cdn.h"
#include "../cJSON/cJSON.h"

static const char Qiniu_CDN_HexadecimalMap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
                                                'E', 'F'};

typedef int(*Qiniu_CDN_NeedToPercentEncode_Fn)(int c);

static int Qiniu_CDN_NeedToPercentEncode(int c) {
    if ('a' <= c) {
        if (c <= 'z' || c == '~') {
            return 0;
        }
        // { | } DEL or chars > 127
        return 1;
    } // if

    if (c <= '9') {
        if ('0' <= c || c == '.' || c == '-') {
            return 0;
        } // if
        return 1;
    } // if

    if ('A' <= c) {
        if (c <= 'Z' || c == '_') {
            return 0;
        }
    } // if
    return 1;
}

static size_t Qiniu_CDN_PercentEncode(char *buf, size_t buf_size, const char *bin, size_t bin_size,
                                      Qiniu_CDN_NeedToPercentEncode_Fn needToPercentEncode) {
    int i = 0;
    int m = 0;
    int ret = 0;

    if (!needToPercentEncode) needToPercentEncode = &Qiniu_CDN_NeedToPercentEncode;

    if (!buf || buf_size <= 0) {
        for (i = 0; i < bin_size; i += 1) {
            if (needToPercentEncode(bin[i])) {
                if (bin[i] == '%' && (i + 2 < bin_size) && isxdigit(bin[i + 1]) && isxdigit(bin[i + 2])) {
                    ret += 1;
                }
                else {
                    ret += 3;
                } // if
            }
            else {
                ret += 1;
            } // if
        } // for
        return ret;
    }
    else if (buf_size < bin_size) {
        return -1;
    } // if

    for (i = 0; i < bin_size; i += 1) {
        if (needToPercentEncode(bin[i])) {
            if (bin[i] == '%' && (i + 2 < bin_size) && isxdigit(bin[i + 1]) && isxdigit(bin[i + 2])) {
                if (m + 1 > buf_size) return -1;
                buf[m++] = bin[i];
            }
            else {
                if (m + 3 > buf_size) return -1;
                buf[m++] = '%';
                buf[m++] = Qiniu_CDN_HexadecimalMap[(bin[i] >> 4) & 0xF];
                buf[m++] = Qiniu_CDN_HexadecimalMap[bin[i] & 0xF];
            } // if
        }
        else {
            if (m + 1 > buf_size) return -1;
            buf[m++] = bin[i];
        }
    } // for
    return m;
}

typedef union _Qiniu_CDN_UnixTime {
    Qiniu_Uint64 tm;
    char bytes[4]; // Only for little-endian architectures.
} Qiniu_CDN_UnixTime;

static int Qiniu_CDN_NeedToPercentEncodeWithoutSlash(int c) {
    if (c == '/') return 0;
    return Qiniu_CDN_NeedToPercentEncode(c);
}

QINIU_DLLAPI char *Qiniu_CDN_MakeDownloadUrlWithDeadline(const char *key, const char *url, Qiniu_Uint64 deadline) {
    int i;
    char *pos;
    char *path;
    size_t pathSize;
    char *encodedPath;
    size_t encodedPathSize;
    char *authedUrl;
    size_t authedUrlSize;
    size_t baseUrlSize;
    char *signStr;
    char *query;
    unsigned char sign[MD5_DIGEST_LENGTH];
    char encodedSign[MD5_DIGEST_LENGTH * 2 + 1];
    char encodedUnixTime[sizeof(Qiniu_CDN_UnixTime) * 2 + 1];
    Qiniu_CDN_UnixTime ut;
    MD5_CTX md5Ctx;

    ut.tm = deadline;
    encodedUnixTime[0] = tolower(Qiniu_CDN_HexadecimalMap[(ut.bytes[3] >> 4) & 0xF]);
    encodedUnixTime[1] = tolower(Qiniu_CDN_HexadecimalMap[ut.bytes[3] & 0xF]);
    encodedUnixTime[2] = tolower(Qiniu_CDN_HexadecimalMap[(ut.bytes[2] >> 4) & 0xF]);
    encodedUnixTime[3] = tolower(Qiniu_CDN_HexadecimalMap[ut.bytes[2] & 0xF]);
    encodedUnixTime[4] = tolower(Qiniu_CDN_HexadecimalMap[(ut.bytes[1] >> 4) & 0xF]);
    encodedUnixTime[5] = tolower(Qiniu_CDN_HexadecimalMap[ut.bytes[1] & 0xF]);
    encodedUnixTime[6] = tolower(Qiniu_CDN_HexadecimalMap[(ut.bytes[0] >> 4) & 0xF]);
    encodedUnixTime[7] = tolower(Qiniu_CDN_HexadecimalMap[ut.bytes[0] & 0xF]);
    encodedUnixTime[8] = '\0';

    pos = strstr(url, "://");
    if (!pos) return NULL;
    path = strchr(pos + 3, '/');
    if (!path) return NULL;

    query = strchr(path, '?');
    if (query) {
        pathSize = query - path;
    }
    else {
        pathSize = strlen(url) - (path - url);
    } // if

    encodedPathSize = Qiniu_CDN_PercentEncode(NULL, -1, path, pathSize, &Qiniu_CDN_NeedToPercentEncodeWithoutSlash);
    encodedPath = malloc(encodedPathSize + 1);
    if (!encodedPath) return NULL;

    Qiniu_CDN_PercentEncode(encodedPath, encodedPathSize, path, pathSize, &Qiniu_CDN_NeedToPercentEncodeWithoutSlash);

    signStr = Qiniu_String_Concat3(key, encodedPath, encodedUnixTime);
    if (!signStr) {
        free(encodedPath);
        return NULL;
    } // if

    MD5_Init(&md5Ctx);
    MD5_Update(&md5Ctx, signStr, strlen(signStr));
    MD5_Final((unsigned char *) sign, &md5Ctx);
    Qiniu_Free(signStr);

    for (i = 0; i < MD5_DIGEST_LENGTH; i += 1) {
        encodedSign[i * 2] = tolower(Qiniu_CDN_HexadecimalMap[(sign[i] >> 4) & 0xF]);
        encodedSign[i * 2 + 1] = tolower(Qiniu_CDN_HexadecimalMap[sign[i] & 0xF]);
    } // if
    encodedSign[MD5_DIGEST_LENGTH * 2] = '\0';

    baseUrlSize = path - url;

    authedUrlSize =
            baseUrlSize + encodedPathSize + 6 + (sizeof(encodedSign) - 1) + 3 + (sizeof(encodedUnixTime) - 1) + 1;
    authedUrl = malloc(authedUrlSize);
    if (!authedUrl) {
        free(encodedPath);
        return NULL;
    } // if

    memcpy(authedUrl, url, baseUrlSize);
    if (query) {
        Qiniu_snprintf(authedUrl + baseUrlSize, authedUrlSize - baseUrlSize, "%s%s&sign=%s&t=%s", encodedPath, query,
                       encodedSign, encodedUnixTime);
    }
    else {
        Qiniu_snprintf(authedUrl + baseUrlSize, authedUrlSize - baseUrlSize, "%s?sign=%s&t=%s", encodedPath,
                       encodedSign, encodedUnixTime);
    } // if

    free(encodedPath);
    return authedUrl;
}

QINIU_DLLAPI Qiniu_Error Qiniu_CDN_RefreshUrls(Qiniu_Client *self, Qiniu_CDN_RefreshRet *ret, char *urls[],
                                               const int urlsCount) {
    Qiniu_Error err;

    char *path = "/v2/tune/refresh";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);
    cJSON *root = cJSON_CreateObject();
    cJSON *pUrls = cJSON_CreateStringArray(urls, urlsCount);
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

QINIU_DLLAPI Qiniu_Error Qiniu_CDN_RefreshDirs(Qiniu_Client *self, Qiniu_CDN_RefreshRet *ret, char *dirs[],
                                               const int dirsCount) {
    Qiniu_Error err;

    char *path = "/v2/tune/refresh";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    cJSON *root = cJSON_CreateObject();
    cJSON *pDirs = cJSON_CreateStringArray(dirs, dirsCount);
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

QINIU_DLLAPI Qiniu_Error Qiniu_CDN_PrefetchUrls(Qiniu_Client *self, Qiniu_CDN_PrefetchRet *ret, char *urls[],
                                                const int num) {
    Qiniu_Error err;

    char *path = "/v2/tune/prefetch";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    cJSON *root = cJSON_CreateObject();
    cJSON *url_a = cJSON_CreateStringArray(urls, num);
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

QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_GetFluxData(
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

    cJSON *root = cJSON_CreateObject();
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

QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_GetBandwidthData(
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

    cJSON *root = cJSON_CreateObject();
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

QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_GetLogList(Qiniu_Client *self, Qiniu_CDN_LogListRet *ret,
                                                     char *domains[], const char *day, const int domainsCount) {
    Qiniu_Error err;

    char *path = "/v2/tune/log/list";
    char *url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    char *domainsStr = Qiniu_String_Join(";", domains, domainsCount);

    cJSON *root = cJSON_CreateObject();
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

QINIU_DLLAPI Qiniu_Error Qiniu_Parse_CDNRefreshRet(Qiniu_Json *root, Qiniu_CDN_RefreshRet *ret) {
    Qiniu_Error err;
    int i;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", NULL);
    ret->requestId = Qiniu_Json_GetString(root, "requestId", NULL);

    cJSON *invalidUrls = cJSON_GetObjectItem(root, "invalidUrls");
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

    cJSON *invalidDirs = cJSON_GetObjectItem(root, "invalidDirs");
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

QINIU_DLLAPI Qiniu_Error Qiniu_Parse_CDNPrefetchRet(Qiniu_Json *root, Qiniu_CDN_PrefetchRet *ret) {
    Qiniu_Error err;
    int i;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", "");
    ret->requestId = Qiniu_Json_GetString(root, "requestId", "");

    cJSON *invalidUrls = cJSON_GetObjectItem(root, "invalidUrls");
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

QINIU_DLLAPI Qiniu_Error Qiniu_Parse_CDNFluxRet(Qiniu_Json *root, Qiniu_CDN_FluxRet *ret, char *domains[],
                                                const int num) {
    Qiniu_Error err;
    int i, j;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", "");
    ret->num = num;
    ret->data_a = (Qiniu_CDN_FluxData *) malloc(sizeof(Qiniu_CDN_FluxData) * num);

    cJSON *tr = cJSON_GetObjectItem(root, "time");
    if (tr == NULL) {
        err.code = 9999;
        err.message = "Failed to parse cdn-flux-ret-time";
        return err;
    }

    cJSON *dr = cJSON_GetObjectItem(root, "data");
    if (dr == NULL) {
        err.code = 9999;
        err.message = "Failed to parse cdn cdn-flux-ret-data";
        return err;
    }

    int count = cJSON_GetArraySize(tr);

    for (i = 0; i < num; ++i) {
        cJSON *item = cJSON_GetObjectItem(dr, domains[i]);
        if (item == NULL) {
            ret->data_a[i].hasValue = FALSE;
            ret->data_a[i].count = 0;
            continue;
        }

        ret->data_a[i].hasValue = TRUE;
        ret->data_a[i].count = count;

        ret->data_a[i].domain = (char *) calloc(256, 1);
        strcpy(ret->data_a[i].domain, domains[i]);

        ret->data_a[i].item_a = (Qiniu_CDN_FluxDataItem *) malloc(sizeof(Qiniu_CDN_FluxDataItem) * count);

        for (j = 0; j < count; ++j) {
            ret->data_a[i].item_a[j].time = (char *) calloc(64, 1);
            strcpy(ret->data_a[i].item_a[j].time, cJSON_GetArrayItem(tr, j)->valuestring);
        }

        cJSON *item_china = cJSON_GetObjectItem(item, "china");
        if (item_china != NULL) {
            for (j = 0; j < count; ++j) {
                cJSON *v = cJSON_GetArrayItem(item_china, j);
                if (v != NULL) {
                    ret->data_a[i].item_a[j].val_china = v->valueint;
                }
                else {
                    ret->data_a[i].item_a[j].val_china = 0;
                }
            }
        }
        else {
            for (j = 0; j < count; ++j) {
                ret->data_a[i].item_a[j].val_china = 0;
            }
        }

        cJSON *item_oversea = cJSON_GetObjectItem(item, "oversea");
        if (item_oversea != NULL) {
            for (j = 0; j < count; ++j) {
                cJSON *v = cJSON_GetArrayItem(item_oversea, j);
                if (v != NULL) {
                    ret->data_a[i].item_a[j].val_oversea = v->valueint;
                }
                else {
                    ret->data_a[i].item_a[j].val_oversea = 0;
                }
            }
        }
        else {
            for (j = 0; j < count; ++j) {
                ret->data_a[i].item_a[j].val_oversea = 0;
            }
        }
    }

    err.code = 200;
    err.message = "OK";

    return err;
}

QINIU_DLLAPI Qiniu_Error Qiniu_Parse_CDNBandwidthRet(Qiniu_Json *root, Qiniu_CDN_BandwidthRet *ret,
                                                     char *domains[], const int num) {
    Qiniu_Error err;
    int i, j;

    ret->code = Qiniu_Json_GetInt(root, "code", 0);
    ret->error = Qiniu_Json_GetString(root, "error", "");
    ret->num = num;
    ret->data_a = (Qiniu_CDN_BandwidthData *) malloc(sizeof(Qiniu_CDN_BandwidthData) * num);

    cJSON *tr = cJSON_GetObjectItem(root, "time");
    if (tr == NULL) {
        err.code = 9999;
        err.message = "Failed to parse cdn time";
        return err;
    }

    cJSON *dr = cJSON_GetObjectItem(root, "data");
    if (dr == NULL) {
        err.code = 9999;
        err.message = "Failed to parse cdn data";
        return err;
    }

    int count = cJSON_GetArraySize(tr);

    for (i = 0; i < num; ++i) {
        cJSON *item = cJSON_GetObjectItem(dr, domains[i]);
        if (item == NULL) {
            ret->data_a[i].hasValue = FALSE;
            ret->data_a[i].count = 0;
            continue;
        }

        ret->data_a[i].hasValue = TRUE;
        ret->data_a[i].count = count;

        ret->data_a[i].domain = (char *) calloc(256, 1);
        strcpy(ret->data_a[i].domain, domains[i]);

        ret->data_a[i].item_a = (Qiniu_CDN_FluxDataItem *) malloc(sizeof(Qiniu_CDN_FluxDataItem) * count);

        for (j = 0; j < count; ++j) {
            ret->data_a[i].item_a[j].time = (char *) calloc(64, 1);
            strcpy(ret->data_a[i].item_a[j].time, cJSON_GetArrayItem(tr, j)->valuestring);
        }

        cJSON *item_china = cJSON_GetObjectItem(item, "china");
        if (item_china != NULL) {
            for (j = 0; j < count; ++j) {
                cJSON *v = cJSON_GetArrayItem(item_china, j);
                if (v != NULL) {
                    ret->data_a[i].item_a[j].val_china = v->valueint;
                }
                else {
                    ret->data_a[i].item_a[j].val_china = 0;
                }
            }
        }
        else {
            for (j = 0; j < count; ++j) {
                ret->data_a[i].item_a[j].val_china = 0;
            }
        }

        cJSON *item_oversea = cJSON_GetObjectItem(item, "oversea");
        if (item_oversea != NULL) {
            for (j = 0; j < count; ++j) {
                cJSON *v = cJSON_GetArrayItem(item_oversea, j);
                if (v != NULL) {
                    ret->data_a[i].item_a[j].val_oversea = v->valueint;
                }
                else {
                    ret->data_a[i].item_a[j].val_oversea = 0;
                }
            }
        }
        else {
            for (j = 0; j < count; ++j) {
                ret->data_a[i].item_a[j].val_oversea = 0;
            }
        }
    }

    err.code = 200;
    err.message = "OK";

    return err;
}

QINIU_DLLAPI Qiniu_Error Qiniu_Parse_CDNLogListRet(Qiniu_Json *root, Qiniu_CDN_LogListRet *ret, char *domains[],
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
        err.message = "no cdn log data";
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

QINIU_DLLAPI void Qiniu_Free_CDNRefreshRet(Qiniu_CDN_RefreshRet *ret) {
    if (ret->invalidUrls != NULL) {
        Qiniu_Free(ret->invalidUrls);
    }

    if (ret->invalidDirs != NULL) {
        Qiniu_Free(ret->invalidDirs);
    }
}

QINIU_DLLAPI void Qiniu_Free_CDNPrefetchRet(Qiniu_CDN_PrefetchRet *ret) {
    if (ret->invalidUrls != NULL) {
        Qiniu_Free(ret->invalidUrls);
    }
}

QINIU_DLLAPI void Qiniu_Free_CDNFluxRet(Qiniu_CDN_FluxRet *ret) {
    int i, j;
    if (ret->error != NULL) {
        free(ret->error);
    }

    if (ret->data_a != NULL) {
        for (i = 0; i < ret->num; ++i) {
            if (ret->data_a[i].hasValue != NULL) {
                free(ret->data_a[i].domain);
                for (j = 0; j < ret->data_a[i].count; ++j) {
                    free(ret->data_a[i].item_a[j].time);
                }
                free(ret->data_a[i].item_a);
            }
        }
        free(ret->data_a);
    }
}

QINIU_DLLAPI void Qiniu_Free_CDNBandwidthRet(Qiniu_CDN_BandwidthRet *ret) {
    int i, j;
    if (ret->error != NULL) {
        free(ret->error);
    }

    if (ret->data_a != NULL) {
        for (i = 0; i < ret->num; ++i) {
            if (ret->data_a[i].hasValue != NULL) {
                free(ret->data_a[i].domain);
                for (j = 0; j < ret->data_a[i].count; ++j) {
                    free(ret->data_a[i].item_a[j].time);
                }
                free(ret->data_a[i].item_a);
            }
        }
        free(ret->data_a);
    }
}

QINIU_DLLAPI void Qiniu_Free_CDNLogListRet(Qiniu_CDN_LogListRet *ret) {
    int i, j;
    if (ret->data != NULL) {
        for (i = 0; i < ret->domainsCount; i++) {
            if (ret->data[i].itemsCount > 0) {
                Qiniu_Free(ret->data[i].items);
            }
        }
        Qiniu_Free(ret->data);
    }
}