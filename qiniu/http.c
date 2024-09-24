/*
 ============================================================================
 Name        : http.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "http.h"
#include "../cJSON/cJSON.h"
#include "../hashmap/hashmap.h"
#include <curl/curl.h>

Qiniu_Error Qiniu_Client_config(Qiniu_Client *self);

/*============================================================================*/
/* type Qiniu_Mutex */

#if defined(_WIN32)

void Qiniu_Mutex_Init(Qiniu_Mutex *self)
{
    InitializeCriticalSection(self);
}

void Qiniu_Mutex_Cleanup(Qiniu_Mutex *self)
{
    DeleteCriticalSection(self);
}

void Qiniu_Mutex_Lock(Qiniu_Mutex *self)
{
    EnterCriticalSection(self);
}

void Qiniu_Mutex_Unlock(Qiniu_Mutex *self)
{
    LeaveCriticalSection(self);
}

#else

void Qiniu_Mutex_Init(Qiniu_Mutex *self)
{
    pthread_mutex_init(self, NULL);
}

void Qiniu_Mutex_Cleanup(Qiniu_Mutex *self)
{
    pthread_mutex_destroy(self);
}

void Qiniu_Mutex_Lock(Qiniu_Mutex *self)
{
    pthread_mutex_lock(self);
}

void Qiniu_Mutex_Unlock(Qiniu_Mutex *self)
{
    pthread_mutex_unlock(self);
}

#endif

/*============================================================================*/
/* Global */

void Qiniu_Buffer_formatInit();

void Qiniu_Global_Init(long flags)
{
    Qiniu_Buffer_formatInit();
    curl_global_init(CURL_GLOBAL_ALL);
}

void Qiniu_Global_Cleanup()
{
    curl_global_cleanup();
}

/*============================================================================*/
/* func Qiniu_call */

static const char g_statusCodeError[] = "http status code is not OK";

Qiniu_Error Qiniu_callex(CURL *curl, Qiniu_Buffer *resp, Qiniu_Json **ret, Qiniu_Bool simpleError,
                         Qiniu_Buffer *resph)
{
    Qiniu_Error err;
    CURLcode curlCode;
    long httpCode;
    Qiniu_Json *root;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Qiniu_Buffer_Fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
    if (resph != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Qiniu_Buffer_Fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, resph);
    }

    curlCode = curl_easy_perform(curl);

    if (curlCode == 0)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (Qiniu_Buffer_Len(resp) != 0)
        {
            root = cJSON_Parse(Qiniu_Buffer_CStr(resp));
        }
        else
        {
            root = NULL;
        }
        *ret = root;
        err.code = (int)httpCode;
        if (httpCode / 100 != 2)
        {
            if (simpleError)
            {
                err.message = g_statusCodeError;
            }
            else
            {
                err.message = Qiniu_Json_GetString(root, "error", g_statusCodeError);
            }
        }
        else
        {
            err.message = "OK";
        }
    }
    else
    {
        *ret = NULL;
        err.code = curlCode;
        err.message = "curl_easy_perform error";
    }

    return err;
}

/*============================================================================*/
/* type Qiniu_Json */

const char *Qiniu_Json_GetString(Qiniu_Json *self, const char *key, const char *defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL && sub->type == cJSON_String)
    {
        return sub->valuestring;
    }
    else
    {
        return defval;
    }
}

const char *Qiniu_Json_GetStringAt(Qiniu_Json *self, int n, const char *defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetArrayItem(self, n);
    if (sub != NULL && sub->type == cJSON_String)
    {
        return sub->valuestring;
    }
    else
    {
        return defval;
    }
}

int Qiniu_Json_GetInt(Qiniu_Json *self, const char *key, int defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL && sub->type == cJSON_Number)
    {
        return (int)sub->valuedouble;
    }
    else
    {
        return defval;
    }
}

Qiniu_Int64 Qiniu_Json_GetInt64(Qiniu_Json *self, const char *key, Qiniu_Int64 defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL && sub->type == cJSON_Number)
    {
        return (Qiniu_Int64)sub->valuedouble;
    }
    else
    {
        return defval;
    }
}

Qiniu_Uint64 Qiniu_Json_GetUInt64(Qiniu_Json *self, const char *key, Qiniu_Uint64 defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL && sub->type == cJSON_Number)
    {
        return (Qiniu_Uint64)sub->valuedouble;
    }
    else
    {
        return defval;
    }
}

int Qiniu_Json_GetBoolean(Qiniu_Json *self, const char *key, int defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    } // if
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL)
    {
        if (sub->type == cJSON_False)
        {
            return 0;
        }
        else if (sub->type == cJSON_True)
        {
            return 1;
        } // if
    }     // if
    return defval;
} // Qiniu_Json_GetBoolean

Qiniu_Json *Qiniu_Json_GetObjectItem(Qiniu_Json *self, const char *key, Qiniu_Json *defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL)
    {
        return sub;
    }
    else
    {
        return defval;
    }
} // Qiniu_Json_GetObjectItem

int Qiniu_Json_GetArraySize(Qiniu_Json *self, const char *key, Qiniu_Int64 defval)
{
    Qiniu_Json *sub;
    int size;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL)
    {
        size = cJSON_GetArraySize(sub);
    }
    else
    {
        size = defval;
    }
    return size;
} // Qiniu_Json_GetArraySize

Qiniu_Json *Qiniu_Json_GetArrayItem(Qiniu_Json *self, int n, Qiniu_Json *defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetArrayItem(self, n);
    if (sub != NULL)
    {
        return sub;
    }
    else
    {
        return defval;
    }
} // Qiniu_Json_GetArrayItem

void Qiniu_Json_Destroy(Qiniu_Json *self)
{
    cJSON_Delete(self);
} // Qiniu_Json_Destroy

Qiniu_Uint32 Qiniu_Json_GetUInt32(Qiniu_Json *self, const char *key, Qiniu_Uint32 defval)
{
    Qiniu_Json *sub;
    if (self == NULL)
    {
        return defval;
    }
    sub = cJSON_GetObjectItem(self, key);
    if (sub != NULL && sub->type == cJSON_Number)
    {
        return (Qiniu_Uint32)sub->valuedouble;
    }
    else
    {
        return defval;
    }
}

/*============================================================================*/
/* type Qiniu_Client */

Qiniu_Auth Qiniu_NoAuth = {
    NULL,
    NULL,
};

void Qiniu_Client_InitEx(Qiniu_Client *self, Qiniu_Auth auth, size_t bufSize)
{
    Qiniu_Zero_Ptr(self);
    self->curl = curl_easy_init();
    self->auth = auth;
    self->hostsRetriesMax = 3;

    Qiniu_Buffer_Init(&self->b, bufSize);
    Qiniu_Buffer_Init(&self->respHeader, bufSize);
}

void Qiniu_Client_InitNoAuth(Qiniu_Client *self, size_t bufSize)
{
    Qiniu_Client_InitEx(self, Qiniu_NoAuth, bufSize);
}

void Qiniu_Client_Cleanup(Qiniu_Client *self)
{
    if (self->auth.itbl != NULL)
    {
        self->auth.itbl->Release(self->auth.self);
        self->auth.itbl = NULL;
    }
    if (self->curl != NULL)
    {
        curl_easy_cleanup((CURL *)self->curl);
        self->curl = NULL;
    }
    if (self->root != NULL)
    {
        cJSON_Delete(self->root);
        self->root = NULL;
    }
    Qiniu_Buffer_Cleanup(&self->b);
    Qiniu_Buffer_Cleanup(&self->respHeader);

    if (self->cachedRegions != NULL)
    {
        hashmap_free(self->cachedRegions);
        self->cachedRegions = NULL;
    }
    self->enableUploadingAcceleration = Qiniu_False;
}

void Qiniu_Client_BindNic(Qiniu_Client *self, const char *nic)
{
    self->boundNic = nic;
} // Qiniu_Client_BindNic

void Qiniu_Client_SetLowSpeedLimit(Qiniu_Client *self, long lowSpeedLimit, long lowSpeedTime)
{
    self->lowSpeedLimit = lowSpeedLimit;
    self->lowSpeedTime = lowSpeedTime;
} // Qiniu_Client_SetLowSpeedLimit

void Qiniu_Client_SetMaximumHostsRetries(Qiniu_Client *self, size_t hostsRetriesMax)
{
    self->hostsRetriesMax = hostsRetriesMax;
} // Qiniu_Client_SetMaximumHostsRetries

void Qiniu_Client_SetTimeout(Qiniu_Client *self, long timeoutMs)
{
    self->timeoutMs = timeoutMs;
} // Qiniu_Client_SetTimeout

void Qiniu_Client_SetConnectTimeout(Qiniu_Client *self, long connectTimeoutMs)
{
    self->connectTimeoutMs = connectTimeoutMs;
} // Qiniu_Client_SetConnectTimeout

void Qiniu_Client_EnableAutoQuery(Qiniu_Client *self, Qiniu_Bool useHttps)
{
    self->autoQueryRegion = Qiniu_True;
    self->autoQueryHttpsRegion = useHttps;
}

void Qiniu_Client_EnableUploadingAcceleration(Qiniu_Client *self)
{
    self->enableUploadingAcceleration = Qiniu_True;
}

void Qiniu_Client_DisableUploadingAcceleration(Qiniu_Client *self)
{
    self->enableUploadingAcceleration = Qiniu_False;
}

void Qiniu_Client_SpecifyRegion(Qiniu_Client *self, Qiniu_Region *region)
{
    self->specifiedRegion = region;
}

CURL *Qiniu_Client_reset(Qiniu_Client *self)
{
    CURL *curl = (CURL *)self->curl;

    curl_easy_reset(curl);
    Qiniu_Buffer_Reset(&self->b);
    Qiniu_Buffer_Reset(&self->respHeader);
    if (self->root != NULL)
    {
        cJSON_Delete(self->root);
        self->root = NULL;
    }

    // Set this option to allow multi-threaded application to get signals, etc
    // Setting CURLOPT_NOSIGNAL to 1 makes libcurl NOT ask the system to ignore SIGPIPE signals
    // See also https://curl.haxx.se/libcurl/c/CURLOPT_NOSIGNAL.html
    // FIXED by fengyh 2017-03-22 10:30
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    return curl;
}

static CURL *Qiniu_Client_initcall_withMethod(Qiniu_Client *self, const char *url, const char *httpMethod)
{
    CURL *curl = Qiniu_Client_reset(self);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, httpMethod);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    return curl;
}

static CURL *Qiniu_Client_initcall(Qiniu_Client *self, const char *url)
{
    return Qiniu_Client_initcall_withMethod(self, url, "POST");
}

static Qiniu_Error Qiniu_Do_Auth(Qiniu_Client *self, const char *method, Qiniu_Header **headers, const char *url, const char *addition, size_t addlen)
{
    if (self->auth.itbl->AuthV2)
    {
        return self->auth.itbl->AuthV2(self->auth.self, method, headers, url, addition, addlen);
    }
    else
    {
        return self->auth.itbl->Auth(self->auth.self, headers, url, addition, addlen);
    }
}

static Qiniu_Error Qiniu_Client_callWithBody(
    Qiniu_Client *self, Qiniu_Json **ret, const char *url,
    const char *body, Qiniu_Int64 bodyLen, const char *mimeType, const char *md5)
{
    int retCode = 0;
    Qiniu_Error err;
    const char *ctxType;
    char ctxLength[64], userAgent[64];
    Qiniu_Header *headers;
    CURL *curl = (CURL *)self->curl;
    err = Qiniu_Client_config(self);
    if (err.code != 200)
    {
        return err;
    }

    curl_easy_setopt(curl, CURLOPT_POST, 1);

    if (mimeType == NULL)
    {
        ctxType = "Content-Type: application/octet-stream";
    }
    else
    {
        ctxType = Qiniu_String_Concat2("Content-Type: ", mimeType);
    }

    Qiniu_snprintf(ctxLength, 64, "Content-Length: %lld", bodyLen);
    Qiniu_snprintf(userAgent, 64, "UserAgent: QiniuC/%s", version);
    headers = curl_slist_append(NULL, ctxLength);
    headers = curl_slist_append(headers, ctxType);
    headers = curl_slist_append(headers, userAgent);
    headers = curl_slist_append(headers, "Expect:");
    if (md5 != NULL)
    {
        char *contentMd5 = Qiniu_String_Concat2("Content-MD5: ", md5);
        headers = curl_slist_append(headers, contentMd5);
    }

    if (self->auth.itbl != NULL)
    {
        if (body == NULL)
        {
            bodyLen = 0;
        }
        err = Qiniu_Do_Auth(self, "POST", &headers, url, body, (size_t)bodyLen);
        if (err.code != 200)
        {
            return err;
        }
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);

    curl_slist_free_all(headers);
    if (mimeType != NULL)
    {
        free((void *)ctxType);
    }

    *ret = self->root;
    return err;
}

Qiniu_Error Qiniu_Client_CallWithMethod(
    Qiniu_Client *self, Qiniu_Json **ret, const char *url,
    Qiniu_Reader body, Qiniu_Int64 bodyLen, const char *mimeType, const char *httpMethod, const char *md5)
{
    return Qiniu_Client_CallWithMethodAndProgressCallback(self, ret, url, body, bodyLen, mimeType, httpMethod, md5, NULL, NULL);
}

Qiniu_Error Qiniu_Client_CallWithMethodAndProgressCallback(
    Qiniu_Client *self, Qiniu_Json **ret, const char *url,
    Qiniu_Reader body, Qiniu_Int64 bodyLen, const char *mimeType, const char *httpMethod, const char *md5,
    int (*callback)(void *, double, double, double, double), void *callbackData)
{
    CURL *curl = Qiniu_Client_initcall_withMethod(self, url, httpMethod);

    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, bodyLen);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, body.Read);
    curl_easy_setopt(curl, CURLOPT_READDATA, body.self);
    if (callback != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, callbackData);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    }

    return Qiniu_Client_callWithBody(self, ret, url, NULL, bodyLen, mimeType, md5);
}

Qiniu_Error Qiniu_Client_CallWithBinary(
    Qiniu_Client *self, Qiniu_Json **ret, const char *url,
    Qiniu_Reader body, Qiniu_Int64 bodyLen, const char *mimeType)
{
    return Qiniu_Client_CallWithBinaryAndProgressCallback(self, ret, url, body, bodyLen, mimeType, NULL, NULL);
}

Qiniu_Error Qiniu_Client_CallWithBinaryAndProgressCallback(
    Qiniu_Client *self, Qiniu_Json **ret, const char *url,
    Qiniu_Reader body, Qiniu_Int64 bodyLen, const char *mimeType,
    int (*callback)(void *, double, double, double, double), void *callbackData)
{
    CURL *curl = Qiniu_Client_initcall(self, url);

    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, bodyLen);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, body.Read);
    curl_easy_setopt(curl, CURLOPT_READDATA, body.self);
    if (callback != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, callbackData);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    }

    return Qiniu_Client_callWithBody(self, ret, url, NULL, bodyLen, mimeType, NULL);
}

Qiniu_Error Qiniu_Client_CallWithBuffer(
    Qiniu_Client *self, Qiniu_Json **ret, const char *url,
    const char *body, size_t bodyLen, const char *mimeType)
{
    CURL *curl = Qiniu_Client_initcall(self, url);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, bodyLen);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    return Qiniu_Client_callWithBody(self, ret, url, body, bodyLen, mimeType, NULL);
}

Qiniu_Error Qiniu_Client_CallWithBuffer2(
    Qiniu_Client *self, Qiniu_Json **ret, const char *url,
    const char *body, size_t bodyLen, const char *mimeType)
{
    CURL *curl = Qiniu_Client_initcall(self, url);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, bodyLen);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    return Qiniu_Client_callWithBody(self, ret, url, NULL, bodyLen, mimeType, NULL);
}

static const char *APPLICATION_WWW_FORM_URLENCODED = "application/x-www-form-urlencoded";

Qiniu_Error Qiniu_Client_Call(Qiniu_Client *self, Qiniu_Json **ret, const char *url)
{
    Qiniu_Error err;
    Qiniu_Header *headers = NULL;
    CURL *curl = Qiniu_Client_initcall(self, url);
    err = Qiniu_Client_config(self);
    if (err.code != 200)
    {
        return err;
    }

    if (self->auth.itbl != NULL)
    {
        err = Qiniu_Do_Auth(self, "POST", &headers, url, NULL, 0);
        if (err.code != 200)
        {
            return err;
        }
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
    curl_slist_free_all(headers);
    *ret = self->root;
    return err;
}

Qiniu_Error Qiniu_Client_CallNoRet(Qiniu_Client *self, const char *url)
{
    Qiniu_Error err;
    Qiniu_Header *headers = NULL;
    CURL *curl = Qiniu_Client_initcall(self, url);
    err = Qiniu_Client_config(self);
    if (err.code != 200)
    {
        return err;
    }

    if (self->auth.itbl != NULL)
    {
        err = Qiniu_Do_Auth(self, "POST", &headers, url, NULL, 0);
        if (err.code != 200)
        {
            return err;
        }
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
    curl_slist_free_all(headers);
    return err;
}
