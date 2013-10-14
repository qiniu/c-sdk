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
#include <curl/curl.h>

#if defined(_WIN32)
#pragma comment(lib, "curllib.lib")
#endif

/*============================================================================*/
/* type Qiniu_Mutex */

#if defined(_WIN32)

void Qiniu_Mutex_Init(Qiniu_Mutex* self)
{
	InitializeCriticalSection(self);
}

void Qiniu_Mutex_Cleanup(Qiniu_Mutex* self)
{
	DeleteCriticalSection(self);
}

void Qiniu_Mutex_Lock(Qiniu_Mutex* self)
{
	EnterCriticalSection(self);
}

void Qiniu_Mutex_Unlock(Qiniu_Mutex* self)
{
	LeaveCriticalSection(self);
}

#else

void Qiniu_Mutex_Init(Qiniu_Mutex* self)
{
	pthread_mutex_init(self, NULL);
}

void Qiniu_Mutex_Cleanup(Qiniu_Mutex* self)
{
	pthread_mutex_destroy(self);
}

void Qiniu_Mutex_Lock(Qiniu_Mutex* self)
{
	pthread_mutex_lock(self);
}

void Qiniu_Mutex_Unlock(Qiniu_Mutex* self)
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

Qiniu_Error Qiniu_callex(CURL* curl, Qiniu_Buffer *resp, Qiniu_Json** ret, Qiniu_Bool simpleError, Qiniu_Buffer *resph)
{
	Qiniu_Error err;
	CURLcode curlCode;
	long httpCode;
	Qiniu_Json* root;

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Qiniu_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
	if (resph != NULL) {
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Qiniu_Buffer_Fwrite);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, resph);
	}

	curlCode = curl_easy_perform(curl);

	if (curlCode == 0) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		if (Qiniu_Buffer_Len(resp) != 0) {
			root = cJSON_Parse(Qiniu_Buffer_CStr(resp));
		} else {
			root = NULL;
		}
		*ret = root;
		err.code = (int)httpCode;
		if (httpCode / 100 != 2) {
			if (simpleError) {
				err.message = g_statusCodeError;
			} else {
				err.message = Qiniu_Json_GetString(root, "error", g_statusCodeError);
			}
		} else {
			err.message = "OK";
		}
	} else {
		*ret = NULL;
		err.code = curlCode;
		err.message = "curl_easy_perform error";
	}

	return err;
}

/*============================================================================*/
/* type Qiniu_Json */

const char* Qiniu_Json_GetString(Qiniu_Json* self, const char* key, const char* defval)
{
	Qiniu_Json* sub;
	if (self == NULL) {
		return defval;
	}
	sub = cJSON_GetObjectItem(self, key);
	if (sub != NULL && sub->type == cJSON_String) {
		return sub->valuestring;
	} else {
		return defval;
	}
}

Qiniu_Int64 Qiniu_Json_GetInt64(Qiniu_Json* self, const char* key, Qiniu_Int64 defval)
{
	Qiniu_Json* sub;
	if (self == NULL) {
		return defval;
	}
	sub = cJSON_GetObjectItem(self, key);
	if (sub != NULL && sub->type == cJSON_Number) {
		return (Qiniu_Int64)sub->valuedouble;
	} else {
		return defval;
	}
}

Qiniu_Uint32 Qiniu_Json_GetInt(Qiniu_Json* self, const char* key, Qiniu_Uint32 defval)
{
	Qiniu_Json* sub;
	if (self == NULL) {
		return defval;
	}
	sub = cJSON_GetObjectItem(self, key);
	if (sub != NULL && sub->type == cJSON_Number) {
		return (Qiniu_Uint32)sub->valueint;
	} else {
		return defval;
	}
}

/*============================================================================*/
/* type Qiniu_Client */

Qiniu_Auth Qiniu_NoAuth = {
	NULL,
	NULL
};

void Qiniu_Client_InitEx(Qiniu_Client* self, Qiniu_Auth auth, size_t bufSize)
{
	self->curl = curl_easy_init();
	self->root = NULL;
	self->auth = auth;

	Qiniu_Buffer_Init(&self->b, bufSize);
	Qiniu_Buffer_Init(&self->respHeader, bufSize);
}

void Qiniu_Client_InitNoAuth(Qiniu_Client* self, size_t bufSize)
{
	Qiniu_Client_InitEx(self, Qiniu_NoAuth, bufSize);
}

void Qiniu_Client_Cleanup(Qiniu_Client* self)
{
	if (self->auth.itbl != NULL) {
		self->auth.itbl->Release(self->auth.self);
		self->auth.itbl = NULL;
	}
	if (self->curl != NULL) {
		curl_easy_cleanup((CURL*)self->curl);
		self->curl = NULL;
	}
	if (self->root != NULL) {
		cJSON_Delete(self->root);
		self->root = NULL;
	}
	Qiniu_Buffer_Cleanup(&self->b);
	Qiniu_Buffer_Cleanup(&self->respHeader);
}

CURL* Qiniu_Client_reset(Qiniu_Client* self)
{
	CURL* curl = (CURL*)self->curl;

	curl_easy_reset(curl);
	Qiniu_Buffer_Reset(&self->b);
	Qiniu_Buffer_Reset(&self->respHeader);
	if (self->root != NULL) {
		cJSON_Delete(self->root);
		self->root = NULL;
	}

	return curl;
}

static CURL* Qiniu_Client_initcall(Qiniu_Client* self, const char* url)
{
	CURL* curl = Qiniu_Client_reset(self);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	return curl;
}

static Qiniu_Error Qiniu_Client_callWithBody(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url, 
    const char* body, Qiniu_Int64 bodyLen, const char* mimeType)
{
	Qiniu_Error err;
	const char* ctxType;
	char ctxLength[64];
	Qiniu_Header* headers = NULL;
	CURL* curl = (CURL*)self->curl;

	curl_easy_setopt(curl, CURLOPT_POST, 1);

	if (mimeType == NULL) {
		ctxType = "Content-Type: application/octet-stream";
	} else {
		ctxType = Qiniu_String_Concat2("Content-Type: ", mimeType);
	}

	Qiniu_snprintf(ctxLength, 64, "Content-Length: %lld", bodyLen);
	headers = curl_slist_append(NULL, ctxLength);
	headers = curl_slist_append(headers, ctxType);

	if (self->auth.itbl != NULL) {
		if (body == NULL) {
			err = self->auth.itbl->Auth(self->auth.self, &headers, url, NULL, 0);
		} else {
			err = self->auth.itbl->Auth(self->auth.self, &headers, url, body, (size_t)bodyLen);
		}

		if (err.code != 200) {
			return err;
		}
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);

	curl_slist_free_all(headers);
	if (mimeType != NULL) {
		free((void*)ctxType);
	}

	*ret = self->root;
	return err;
}

Qiniu_Error Qiniu_Client_CallWithBinary(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	Qiniu_Reader body, Qiniu_Int64 bodyLen, const char* mimeType)
{
	CURL* curl = Qiniu_Client_initcall(self, url);

	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, bodyLen);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, body.Read);
	curl_easy_setopt(curl, CURLOPT_READDATA, body.self);

	return Qiniu_Client_callWithBody(self, ret, url, NULL, bodyLen, mimeType);
}

Qiniu_Error Qiniu_Client_CallWithBuffer(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	const char* body, size_t bodyLen, const char* mimeType)
{
	CURL* curl = Qiniu_Client_initcall(self, url);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, bodyLen);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

	return Qiniu_Client_callWithBody(self, ret, url, body, bodyLen, mimeType);
}

Qiniu_Error Qiniu_Client_Call(Qiniu_Client* self, Qiniu_Json** ret, const char* url)
{
	Qiniu_Error err;
	Qiniu_Header* headers = NULL;
	CURL* curl = Qiniu_Client_initcall(self, url);

	if (self->auth.itbl != NULL) {
		err = self->auth.itbl->Auth(self->auth.self, &headers, url, NULL, 0);
		if (err.code != 200) {
			return err;
		}
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


	err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
	/*
	 * Bug No.(4601) Wang Xiaotao 2013\10\12 17:09:02
	 * Change for : free  var headers 'variable'
	 * Reason     : memory leak!
	 */
    curl_slist_free_all(headers);
	*ret = self->root;
	return err;
}

Qiniu_Error Qiniu_Client_CallNoRet(Qiniu_Client* self, const char* url)
{
	Qiniu_Error err;
	Qiniu_Header* headers = NULL;
	CURL* curl = Qiniu_Client_initcall(self, url);

	if (self->auth.itbl != NULL) {
		err = self->auth.itbl->Auth(self->auth.self, &headers, url, NULL, 0);
		if (err.code != 200) {
			return err;
		}
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	/*
	 * Bug No.(4601) Wang Xiaotao 2013\10\12 17:09:02
	 * Change for : free  var headers 'variable'
	 * Reason     : memory leak!
	 */
	err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
	curl_slist_free_all(headers); 
	return err;
}

