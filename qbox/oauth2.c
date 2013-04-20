/*
 ============================================================================
 Name        : oauth2.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "oauth2.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

/*============================================================================*/
/* type QBox_Mutex */

#if defined(_WIN32)

void QBox_Mutex_Init(QBox_Mutex* self)
{
	InitializeCriticalSection(self);
}

void QBox_Mutex_Cleanup(QBox_Mutex* self)
{
	DeleteCriticalSection(self);
}

void QBox_Mutex_Lock(QBox_Mutex* self)
{
	EnterCriticalSection(self);
}

void QBox_Mutex_Unlock(QBox_Mutex* self)
{
	LeaveCriticalSection(self);
}

#else

void QBox_Mutex_Init(QBox_Mutex* self)
{
	pthread_mutex_init(self, NULL);
}

void QBox_Mutex_Cleanup(QBox_Mutex* self)
{
	pthread_mutex_destroy(self);
}

void QBox_Mutex_Lock(QBox_Mutex* self)
{
	pthread_mutex_lock(self);
}

void QBox_Mutex_Unlock(QBox_Mutex* self)
{
	pthread_mutex_unlock(self);
}

#endif

/*============================================================================*/
/* Global */

void QBox_Global_Init(long flags)
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void QBox_Global_Cleanup()
{
	curl_global_cleanup();
}

/*============================================================================*/
/* func QBox_call */

static const char g_statusCodeError[] = "http status code is not OK";

QBox_Error QBox_callex(CURL* curl, QBox_Buffer *resp, QBox_Json** ret, QBox_Bool simpleError, QBox_Buffer *resph)
{
	QBox_Error err = {};
	CURLcode curlCode;
	long httpCode;
	QBox_Json* root;

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBox_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
	if (resph != NULL) {
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, QBox_Buffer_Fwrite);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, resph);
	}

	curlCode = curl_easy_perform(curl);

	if (curlCode == 0) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		if (QBox_Buffer_Len(resp) != 0) {
			root = cJSON_Parse(QBox_Buffer_CStr(resp));
		} else {
			root = NULL;
		}
		*ret = root;
		err.code = (int)httpCode;
		if (httpCode / 100 != 2) {
			if (simpleError) {
				err.message = g_statusCodeError;
			} else {
				err.message = QBox_Json_GetString(root, "error", g_statusCodeError);
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
/* type QBox_Json */

const char* QBox_Json_GetString(QBox_Json* self, const char* key, const char* defval)
{
	QBox_Json* sub;
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

QBox_Int64 QBox_Json_GetInt64(QBox_Json* self, const char* key, QBox_Int64 defval)
{
	QBox_Json* sub;
	if (self == NULL) {
		return defval;
	}
	sub = cJSON_GetObjectItem(self, key);
	if (sub != NULL && sub->type == cJSON_Number) {
		return (QBox_Int64)sub->valuedouble;
	} else {
		return defval;
	}
}

/*============================================================================*/
/* type QBox_Client */

static QBox_Auth QBox_NoAuth = {
	NULL,
	NULL
};

void QBox_Client_InitEx(QBox_Client* self, QBox_Auth auth, size_t bufSize)
{
	self->curl = curl_easy_init();
	self->root = NULL;
	self->auth = auth;

	QBox_Buffer_Init(&self->b, bufSize);
	QBox_Buffer_Init(&self->respHeader, bufSize);
}

void QBox_Client_InitNoAuth(QBox_Client* self, size_t bufSize)
{
	QBox_Client_InitEx(self, QBox_NoAuth, bufSize);
}

void QBox_Client_Cleanup(QBox_Client* self)
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
	QBox_Buffer_Cleanup(&self->b);
	QBox_Buffer_Cleanup(&self->respHeader);
}

CURL* QBox_Client_reset(QBox_Client* self)
{
	CURL* curl = (CURL*)self->curl;

	curl_easy_reset(curl);
	QBox_Buffer_Reset(&self->b);
	QBox_Buffer_Reset(&self->respHeader);
	if (self->root != NULL) {
		cJSON_Delete(self->root);
		self->root = NULL;
	}

	return curl;
}

static void QBox_Client_initcall(QBox_Client* self, const char* url)
{
	CURL* curl = QBox_Client_reset(self);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_URL, url);
}

static QBox_Error QBox_Client_callWithBody(
	QBox_Client* self, QBox_Json** ret, const char* url, QBox_Int64 bodyLen,
    CURL* curl, struct curl_slist* headers)
{
	QBox_Error err;
	char ctxLength[64];

	curl_easy_setopt(curl, CURLOPT_POST, 1);

	QBox_snprintf(ctxLength, 64, "Content-Length: %lld", bodyLen);
	headers = curl_slist_append(NULL, ctxLength);
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

	if (self->auth.itbl != NULL) {
		err = self->auth.itbl->Auth(self->auth.self, &headers, url, NULL, 0);
		if (err.code != 200) {
			return err;
		}
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	err = QBox_callex(curl, &self->b, &self->root, QBox_False, &self->respHeader);

	curl_slist_free_all(headers);

	*ret = self->root;
	return err;
}

QBox_Error QBox_Client_CallWithBinary(
	QBox_Client* self, QBox_Json** ret, const char* url, QBox_Reader body, QBox_Int64 bodyLen)
{
	CURL* curl;
	struct curl_slist* headers;
	QBox_Error err;

	QBox_Client_initcall(self, url);

	curl = (CURL*)self->curl;
	curl_easy_setopt(curl, CURLOPT_INFILESIZE, bodyLen);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, body.Read);
	curl_easy_setopt(curl, CURLOPT_READDATA, body.self);

	return QBox_Client_callWithBody(self, ret, url, bodyLen, curl, headers);
}

QBox_Error QBox_Client_CallWithBuffer(
	QBox_Client* self, QBox_Json** ret, const char* url, const char* body, QBox_Int64 bodyLen)
{
	CURL* curl;
	struct curl_slist* headers;
	QBox_Error err;

	QBox_Client_initcall(self, url);

	curl = (CURL*)self->curl;
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, bodyLen);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

	return QBox_Client_callWithBody(self, ret, url, bodyLen, curl, headers);
}

QBox_Error QBox_Client_Call(QBox_Client* self, QBox_Json** ret, const char* url)
{
	QBox_Error err;
	QBox_Header* headers = NULL;
	CURL* curl = (CURL*)self->curl;

	QBox_Client_initcall(self, url);

	if (self->auth.itbl != NULL) {
		err = self->auth.itbl->Auth(self->auth.self, &headers, url, NULL, 0);
		if (err.code != 200) {
			return err;
		}
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	err = QBox_callex(curl, &self->b, &self->root, QBox_False, &self->respHeader);
	*ret = self->root;
	return err;
}

QBox_Error QBox_Client_CallNoRet(QBox_Client* self, const char* url)
{
	QBox_Error err;
	QBox_Header* headers = NULL;
	CURL* curl = (CURL*)self->curl;

	QBox_Client_initcall(self, url);

	if (self->auth.itbl != NULL) {
		err = self->auth.itbl->Auth(self->auth.self, &headers, url, NULL, 0);
		if (err.code != 200) {
			return err;
		}
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	return QBox_callex(curl, &self->b, &self->root, QBox_False, &self->respHeader);
}

