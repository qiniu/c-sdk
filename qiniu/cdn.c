#include <openssl/md5.h>
#include <ctype.h>
#include "base.h"
#include "cdn.h"
#include "../cJSON/cJSON.h"

static const char Qiniu_Cdn_HexadecimalMap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

typedef int (*Qiniu_Cdn_NeedToPercentEncode_Fn)(int c);

static int Qiniu_Cdn_NeedToPercentEncode(int c)
{
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

static size_t Qiniu_Cdn_PercentEncode(char * buf, size_t buf_size, const char * bin, size_t bin_size, Qiniu_Cdn_NeedToPercentEncode_Fn needToPercentEncode)
{
    int i = 0;
    int m = 0;
    int ret = 0;

    if (! needToPercentEncode) needToPercentEncode = &Qiniu_Cdn_NeedToPercentEncode;

    if (!buf || buf_size <= 0) {
        for (i = 0; i < bin_size; i += 1) {
            if (needToPercentEncode(bin[i])) {
                if (bin[i] == '%' && (i + 2 < bin_size) && isxdigit(bin[i+1]) && isxdigit(bin[i+2])) {
                    ret += 1;
                } else {
                    ret += 3;
                } // if
            } else {
                ret += 1;
            } // if
        } // for
        return ret;
    } else if (buf_size < bin_size) {
        return -1;
    } // if

    for (i = 0; i < bin_size; i += 1) {
        if (needToPercentEncode(bin[i])) {
            if (bin[i] == '%' && (i + 2 < bin_size) && isxdigit(bin[i+1]) && isxdigit(bin[i+2])) {
                    if (m + 1 > buf_size) return -1;
                    buf[m++] = bin[i];
            } else {
                if (m + 3 > buf_size) return -1;
                buf[m++] = '%';
                buf[m++] = Qiniu_Cdn_HexadecimalMap[(bin[i] >> 4) & 0xF];
                buf[m++] = Qiniu_Cdn_HexadecimalMap[bin[i] & 0xF];
            } // if
        } else {
            if (m + 1 > buf_size) return -1;
            buf[m++] = bin[i];
        }
    } // for
    return m;
}

typedef union _Qiniu_Cdn_UnixTime
{
    Qiniu_Uint64 tm;
    char bytes[4]; // Only for little-endian architectures.
} Qiniu_Cdn_UnixTime;

static int Qiniu_Cdn_NeedToPercentEncodeWithoutSlash(int c)
{
    if (c == '/') return 0;
    return Qiniu_Cdn_NeedToPercentEncode(c);
}

QINIU_DLLAPI char * Qiniu_Cdn_MakeDownloadUrlWithDeadline(const char * key, const char * url, Qiniu_Uint64 deadline)
{
    int i;
    char * pos;
    char * path;
    size_t pathSize;
    char * encodedPath;
    size_t encodedPathSize;
    char * authedUrl;
    size_t authedUrlSize;
    size_t baseUrlSize;
    char * signStr;
    char * query;
    unsigned char sign[MD5_DIGEST_LENGTH];
    char encodedSign[MD5_DIGEST_LENGTH * 2 + 1];
    char encodedUnixTime[sizeof(Qiniu_Cdn_UnixTime) * 2 + 1];
    Qiniu_Cdn_UnixTime ut;
    MD5_CTX md5Ctx;

    ut.tm = deadline;
    encodedUnixTime[0] = tolower(Qiniu_Cdn_HexadecimalMap[(ut.bytes[3] >> 4) & 0xF]);
    encodedUnixTime[1] = tolower(Qiniu_Cdn_HexadecimalMap[ut.bytes[3] & 0xF]);
    encodedUnixTime[2] = tolower(Qiniu_Cdn_HexadecimalMap[(ut.bytes[2] >> 4) & 0xF]);
    encodedUnixTime[3] = tolower(Qiniu_Cdn_HexadecimalMap[ut.bytes[2] & 0xF]);
    encodedUnixTime[4] = tolower(Qiniu_Cdn_HexadecimalMap[(ut.bytes[1] >> 4) & 0xF]);
    encodedUnixTime[5] = tolower(Qiniu_Cdn_HexadecimalMap[ut.bytes[1] & 0xF]);
    encodedUnixTime[6] = tolower(Qiniu_Cdn_HexadecimalMap[(ut.bytes[0] >> 4) & 0xF]);
    encodedUnixTime[7] = tolower(Qiniu_Cdn_HexadecimalMap[ut.bytes[0] & 0xF]);
    encodedUnixTime[8] = '\0';

    pos = strstr(url, "://"); 
    if (! pos) return NULL;
    path = strchr(pos + 3, '/');
    if (! path) return NULL;

    query = strchr(path, '?');
    if (query) {
        pathSize = query - path;
    } else {
        pathSize = strlen(url) - (path - url);
    } // if

    encodedPathSize = Qiniu_Cdn_PercentEncode(NULL, -1, path, pathSize, &Qiniu_Cdn_NeedToPercentEncodeWithoutSlash);
    encodedPath = malloc(encodedPathSize + 1);
    if (! encodedPath) return NULL;

    Qiniu_Cdn_PercentEncode(encodedPath, encodedPathSize, path, pathSize, &Qiniu_Cdn_NeedToPercentEncodeWithoutSlash);

    signStr = Qiniu_String_Concat3(key, encodedPath, encodedUnixTime);
    if (! signStr) {
        free(encodedPath);
        return NULL;
    } // if

    MD5_Init(&md5Ctx);
    MD5_Update(&md5Ctx, signStr, strlen(signStr));
    MD5_Final((unsigned char *)sign, &md5Ctx);
    Qiniu_Free(signStr);

    for (i = 0; i < MD5_DIGEST_LENGTH; i += 1) {
        encodedSign[i * 2] = tolower(Qiniu_Cdn_HexadecimalMap[(sign[i] >> 4) & 0xF]);
        encodedSign[i * 2 + 1] = tolower(Qiniu_Cdn_HexadecimalMap[sign[i] & 0xF]);
    } // if
    encodedSign[MD5_DIGEST_LENGTH * 2] = '\0';
    
    baseUrlSize = path - url;

    authedUrlSize = baseUrlSize + encodedPathSize + 6 + (sizeof(encodedSign) - 1) + 3 + (sizeof(encodedUnixTime) - 1) + 1;
    authedUrl = malloc(authedUrlSize);
    if (! authedUrl) {
        free(encodedPath);
        return NULL;
    } // if

    memcpy(authedUrl, url, baseUrlSize);
    if (query) {
        Qiniu_snprintf(authedUrl + baseUrlSize, authedUrlSize - baseUrlSize, "%s%s&sign=%s&t=%s", encodedPath, query, encodedSign, encodedUnixTime);
    } else {
        Qiniu_snprintf(authedUrl + baseUrlSize, authedUrlSize - baseUrlSize, "%s?sign=%s&t=%s", encodedPath, encodedSign, encodedUnixTime);
    } // if

    free(encodedPath);
    return authedUrl;
}

// TODO: Parse `ret->invalidUrls` and `ret->invalidDirs` fields
QINIU_DLLAPI Qiniu_Error Qiniu_Cdn_RefreshUrls(Qiniu_Client*self, Qiniu_Cdn_RefreshRet* ret, const char* urls[], const int num)
{  
    Qiniu_Error err;

    char* path = "/v2/tune/refresh";
    char* url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    cJSON* root = cJSON_CreateObject();
    cJSON* url_a = cJSON_CreateStringArray(urls,num);    
    cJSON_AddItemToObject(root,"urls",url_a);
    //cJSON_AddItemToObject(root,"dirs",cJSON_CreateStringArray(NULL,0));
    char* body = cJSON_PrintUnformatted(root);

    Qiniu_Json* jsonRet = NULL;   
    
    err = Qiniu_Client_CallWithBuffer2(
        self,&jsonRet,url,body,strlen(body),"application/json");
    
    if (err.code == 200) {
		ret->code = Qiniu_Json_GetInt(jsonRet, "code", 0);
        ret->error = Qiniu_Json_GetString(jsonRet, "error", "");
        ret->requestId = Qiniu_Json_GetString(jsonRet, "requestId", "");
        //ret->invalidUrls = Qiniu_Json_GetObjectItem(jsonRet, "invalidUrls", "");
        //ret->invalidDirs = Qiniu_Json_GetObjectItem(jsonRet, "invalidDirs", "");
        ret->urlQuotaDay = Qiniu_Json_GetInt(jsonRet, "urlQuotaDay", 0);
        ret->urlSurplusDay = Qiniu_Json_GetInt(jsonRet, "urlSurplusDay", 0);
        ret->dirQuotaDay = Qiniu_Json_GetInt(jsonRet, "dirQuotaDay", 0);
        ret->dirSurplusDay = Qiniu_Json_GetInt(jsonRet, "dirSurplusDay", 0);
	}

    Qiniu_Free(url);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

// TODO: Parse `ret->invalidUrls` and `ret->invalidDirs` fields
QINIU_DLLAPI Qiniu_Error Qiniu_Cdn_RefreshDirs(Qiniu_Client*self, Qiniu_Cdn_RefreshRet* ret, const char* dirs[], const int num)
{  
    Qiniu_Error err;

    char* path = "/v2/tune/refresh";
    char* url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    cJSON* root = cJSON_CreateObject();
    cJSON* dir_a = cJSON_CreateStringArray(dirs,num);
    //cJSON_AddItemToObject(root,"urls",cJSON_CreateStringArray(NULL,0));
    cJSON_AddItemToObject(root,"dirs",dir_a); 
    char* body = cJSON_PrintUnformatted(root);

    Qiniu_Json* jsonRet = NULL;   

    err = Qiniu_Client_CallWithBuffer2(
        self, &jsonRet,url,body,strlen(body),"application/json");
    
    if (err.code == 200) {
		ret->code = Qiniu_Json_GetInt(jsonRet, "code", 0);
        ret->error = Qiniu_Json_GetString(jsonRet, "error", "");
        ret->requestId = Qiniu_Json_GetString(jsonRet, "requestId", "");
        //ret->invalidUrls = Qiniu_Json_GetObjectItem(jsonRet, "invalidUrls", "");
        //ret->invalidDirs = Qiniu_Json_GetObjectItem(jsonRet, "invalidDirs", "");
        ret->urlQuotaDay = Qiniu_Json_GetInt(jsonRet, "urlQuotaDay", 0);
        ret->urlSurplusDay = Qiniu_Json_GetInt(jsonRet, "urlSurplusDay", 0);
        ret->dirQuotaDay = Qiniu_Json_GetInt(jsonRet, "dirQuotaDay", 0);
        ret->dirSurplusDay = Qiniu_Json_GetInt(jsonRet, "dirSurplusDay", 0);        
	}

    Qiniu_Free(url);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

// TODO: Parse `ret->invalidUrls` field
QINIU_DLLAPI Qiniu_Error Qiniu_Cdn_PrefetchUrls(Qiniu_Client*self, Qiniu_Cdn_PrefetchRet* ret, const char* urls[], const int num)
{  
    Qiniu_Error err;

    char* path = "/v2/tune/prefetch";
    char* url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

    cJSON* root = cJSON_CreateObject();
    cJSON* url_a = cJSON_CreateStringArray(urls,num);
    cJSON_AddItemToObject(root,"urls",url_a);   
    char* body = cJSON_PrintUnformatted(root);

    Qiniu_Json* jsonRet = NULL;   

    err = Qiniu_Client_CallWithBuffer2(
        self,&jsonRet,url,body,strlen(body),"application/json");

    
    if (err.code == 200) {
		ret->code = Qiniu_Json_GetInt(jsonRet, "code", 0);
        ret->error = Qiniu_Json_GetString(jsonRet, "error", "");
        ret->requestId = Qiniu_Json_GetString(jsonRet, "requestId", "");
        //ret->invalidUrls = Qiniu_Json_GetObjectItem(jsonRet, "invalidUrls", "");
        ret->quotaDay = Qiniu_Json_GetInt(jsonRet, "urlQuotaDay", 0);
        ret->surplusDay = Qiniu_Json_GetInt(jsonRet, "urlSurplusDay", 0);
	}

    Qiniu_Free(url);
    Qiniu_Free(body);
    cJSON_Delete(root);

    return err;
}

// TODO: Parse `ret->time` and `ret->data` fields
QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetFluxData(
    Qiniu_Client* self, 
    Qiniu_Cdn_FluxBandwidthRet* ret,
    const char* startDate,
    const char* endDate,
    const char* granularity,
    const char* domains[], 
    const int num)
 {
     Qiniu_Error err;

     char* path = "/v2/tune/flux";
     char* url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

     char* domains_str = Qiniu_String_Join(";",domains,num);

     cJSON* root = cJSON_CreateObject();
     cJSON_AddStringToObject(root,"startDate",startDate);   
     cJSON_AddStringToObject(root,"endDate",endDate);
     cJSON_AddStringToObject(root,"granularity",granularity);    
     cJSON_AddStringToObject(root,"domains",domains_str);
     char* body = cJSON_PrintUnformatted(root);

     Qiniu_Json* jsonRet = NULL;   

     err = Qiniu_Client_CallWithBuffer2(
         self,&jsonRet,url,body,strlen(body),"application/json");

    
     if (err.code == 200) {
		 ret->code = Qiniu_Json_GetInt(jsonRet, "code", 0);
         ret->error = Qiniu_Json_GetString(jsonRet, "error", "");
         //ret->time = Qiniu_Json_GetString(jsonRet, "time", "");
         //ret->data = Qiniu_Json_GetObjectItem(jsonRet, "data", "");
	 }

     Qiniu_Free(url);
     //Qiniu_Free(domains_str);
     Qiniu_Free(body);
     cJSON_Delete(root);

     return err;
 }

// TODO: Parse `ret->time` and `ret->data` fields
QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetBandwidthData(
    Qiniu_Client* self,
    Qiniu_Cdn_FluxBandwidthRet* ret, 
    const char* startDate,
    const char* endDate,
    const char* granularity,
    const char* domains[],
    const int num)
{
     Qiniu_Error err;

     char* path = "/v2/tune/bandwidth";
     char* url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

     char* domains_str = Qiniu_String_Join(";",domains,num);

     cJSON* root = cJSON_CreateObject();
     cJSON_AddStringToObject(root,"startDate",startDate);   
     cJSON_AddStringToObject(root,"endDate",endDate);
     cJSON_AddStringToObject(root,"granularity",granularity);    
     cJSON_AddStringToObject(root,"domains",domains_str);
     char* body = cJSON_PrintUnformatted(root);

     Qiniu_Json* jsonRet = NULL;   

     err = Qiniu_Client_CallWithBuffer2(
         self,&jsonRet,url,body,strlen(body),"application/json");

    
     if (err.code == 200) {
		 ret->code = Qiniu_Json_GetInt(jsonRet, "code", 0);
         ret->error = Qiniu_Json_GetString(jsonRet, "error", "");
         //ret->time = Qiniu_Json_GetString(jsonRet, "time", "");
         //ret->data = Qiniu_Json_GetObjectItem(jsonRet, "data", "");
	 }

     Qiniu_Free(url);
     Qiniu_Free(domains_str);
     Qiniu_Free(body);
     cJSON_Delete(root);

     return err;
}

// TODO: Parse `ret->data` field
QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetLogList(
    Qiniu_Client* self,
    Qiniu_Cdn_LogListRet* ret,
    const char* day,
    const char* domains[],
    const int num)
{
     Qiniu_Error err;

     char* path = "/v2/tune/log/list";
     char* url = Qiniu_String_Concat2(QINIU_FUSION_HOST, path);

     char* domains_str = Qiniu_String_Join(";",domains,num);

     cJSON* root = cJSON_CreateObject();
     cJSON_AddStringToObject(root,"day",day);      
     cJSON_AddStringToObject(root,"domains",domains_str);
     char* body = cJSON_PrintUnformatted(root);

     Qiniu_Json* jsonRet = NULL;   

     err = Qiniu_Client_CallWithBuffer2(
         self,&jsonRet,url,body,strlen(body),"application/json");

    
     if (err.code == 200) {
		 ret->code = Qiniu_Json_GetInt(jsonRet, "code", 0);
         ret->error = Qiniu_Json_GetString(jsonRet, "error", "");
         //ret->data = Qiniu_Json_GetObjectItem(jsonRet, "data", "");
	 }

     Qiniu_Free(url);
     Qiniu_Free(domains_str);
     Qiniu_Free(body);
     cJSON_Delete(root);

     return err;
}