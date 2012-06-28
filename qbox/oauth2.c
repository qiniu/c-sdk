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
#include <openssl/hmac.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

#define QBox_initBufSize	1024

struct QBox_Virtual_Client_FuncTable QBox_Client_Default_Func = {
    NULL, /* QBox_Client_CallWithBinary */
    NULL, /* QBox_Client_Call */
    NULL  /* QBox_Client_CallNoRet */
};

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

static QBox_Error QBox_callex(CURL* curl, QBox_Buffer *resp, QBox_Json** ret, QBox_Bool simpleError)
{
	QBox_Error err;
	CURLcode curlCode;
	long httpCode;
	QBox_Json* root;

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBox_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);

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

static QBox_Error QBox_call(CURL* curl, int bufSize, QBox_Json** ret, QBox_Bool simpleError)
{
	QBox_Error err;
	QBox_Buffer resp;
	QBox_Buffer_Init(&resp, bufSize);

	err = QBox_callex(curl, &resp, ret, simpleError);

	QBox_Buffer_Cleanup(&resp);
	return err;
}

/*============================================================================*/
/* type QBox_Token */

static void QBox_Token_set(QBox_Token* self, const char* accessToken, const char* refreshToken, QBox_Int64 expiry)
{
	size_t accessTokenLen = strlen(accessToken) + 1;
	size_t refreshTokenLen = strlen(refreshToken) + 1;
	char* accessTokenPtr = (char*)malloc(accessTokenLen + refreshTokenLen);
	char* refreshTokenPtr = accessTokenPtr + accessTokenLen;
	if (self->accessToken != NULL) {
		free((void*)self->accessToken);
	}
	self->accessToken = accessTokenPtr;
	self->refreshToken = refreshTokenPtr;
	self->expiry = expiry;
	memcpy(accessTokenPtr, accessToken, accessTokenLen);
	memcpy(refreshTokenPtr, refreshToken, refreshTokenLen);
}

QBox_Count QBox_Token_Acquire(QBox_Token* self)
{
	return QBox_Count_Inc(&self->nref);
}

QBox_Count QBox_Token_Release(QBox_Token* self)
{
	QBox_Count ret = QBox_Count_Dec(&self->nref);
	if (ret == 0) {
		if (self->accessToken != NULL) {
			free((void*)self->accessToken);
		}
		QBox_Mutex_Cleanup(&self->mutex);
		free(self);
	}
	return ret;
}

QBox_Token* QBox_Token_new()
{
	QBox_Token* self = (QBox_Token*)malloc(sizeof(QBox_Token));
	self->nref = 1;
	self->accessToken = NULL;
	QBox_Mutex_Init(&self->mutex);
	return self;
}

QBox_Token* QBox_Token_New(const char* accessToken, const char* refreshToken, QBox_Int64 expiry)
{
	QBox_Token* self = QBox_Token_new();
	QBox_Token_set(self, accessToken, refreshToken, expiry);
	return self;
}

char* QBox_Token_Access(QBox_Token* self, QBox_Int64* expiry)
{
	char* accessToken;

	QBox_Mutex_Lock(&self->mutex);
	accessToken = strdup(self->accessToken);
	*expiry = self->expiry;
	QBox_Mutex_Unlock(&self->mutex);

	return accessToken;
}

static void QBox_Token_ret(QBox_Token* self, QBox_Json* root, QBox_Error* err)
{
	QBox_Int64 expiry;
	const char* accessToken = QBox_Json_GetString(root, "access_token", NULL);
	const char* refreshToken = QBox_Json_GetString(root, "refresh_token", NULL);

	if (accessToken == NULL || refreshToken == NULL) {
		err->code = 9998;
		err->message = "unexcepted response";
	} else {
		expiry = QBox_Seconds() + QBox_Json_GetInt64(root, "expires_in", 0);
		QBox_Token_set(self, accessToken, refreshToken, expiry);
	}
}

static QBox_Error QBox_Token_refresh(QBox_Token* self, const char* refreshToken)
{
	QBox_Error err;
	CURL* curl;
	QBox_Json* root = NULL;

	char* params = QBox_String_Concat(
		"client_id=", QBOX_CLIENT_ID,
		"&client_secret=", QBOX_CLIENT_SECRET,
		"&grant_type=refresh_token&refresh_token=", refreshToken, NULL);

	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_URL, QBOX_TOKEN_ENDPOINT);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);

	err = QBox_call(curl, QBox_initBufSize, &root, QBox_True);
	curl_easy_cleanup(curl);
	free(params);

	if (err.code == 200) {
		QBox_Token_ret(self, root, &err);
	}

	if (root != NULL) {
		cJSON_Delete(root);
	}
	return err;
}

QBox_Error QBox_Token_ExchangeByPassword(QBox_Token** token, const char* user, const char* passwd)
{
	char* params;
	char* userEsc;
	char* passwdEsc;
	int fuserEsc, fpasswdEsc;
	CURL* curl;
	QBox_Error err;
	QBox_Json* root = NULL;

	userEsc = QBox_QueryEscape(user, &fuserEsc);
	passwdEsc = QBox_QueryEscape(passwd, &fpasswdEsc);

	params = QBox_String_Concat(
		"client_id=", QBOX_CLIENT_ID, "&client_secret=", QBOX_CLIENT_SECRET,
		"&grant_type=password&username=", userEsc, "&password=", passwdEsc, NULL);

	if (fuserEsc) {
		free(userEsc);
	}
	if (fpasswdEsc) {
		free(passwdEsc);
	}

	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_URL, QBOX_TOKEN_ENDPOINT);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);

	err = QBox_call(curl, QBox_initBufSize, &root, QBox_True);
	curl_easy_cleanup(curl);
	free(params);

	if (err.code == 200) {
		*token = QBox_Token_new();
		QBox_Token_ret(*token, root, &err);
		if (err.code != 200) {
			QBox_Token_Release(*token);
		}
	}

	if (root != NULL) {
		cJSON_Delete(root);
	}
	return err;
}

QBox_Error QBox_Token_ExchangeByRefreshToken(QBox_Token** token, const char* refreshToken)
{
	QBox_Error err;
	*token = QBox_Token_new();
	err = QBox_Token_refresh(*token, refreshToken);
	if (err.code != 200) {
		QBox_Token_Release(*token);
	}
	return err;
}

QBox_Error QBox_Token_Refresh(QBox_Token* self)
{
	QBox_Error err;
	QBox_Mutex_Lock(&self->mutex);
	err = QBox_Token_refresh(self, self->refreshToken);
	QBox_Mutex_Unlock(&self->mutex);
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

static void QBox_Client_initAccess(QBox_Client* self)
{
	char* accessToken;
	char* auth;

	accessToken = QBox_Token_Access(self->token, &self->expiry);
	auth = QBox_String_Concat2("Authorization: Bearer ", accessToken);
	free(accessToken);

	self->authHeader = curl_slist_append(NULL, auth);
	free(auth);
}

void QBox_Client_Init(QBox_Client* self, QBox_Token* token, size_t bufSize)
{
	self->curl = curl_easy_init();
	self->token = token;
	self->root = NULL;
    self->refreshToken = 1;

	QBox_Buffer_Init(&self->b, bufSize);
	QBox_Token_Acquire(token);
	QBox_Client_initAccess(self);

    self->vptr = &QBox_Client_Default_Func;
}

void QBox_Client_Cleanup(QBox_Client* self)
{
	QBox_Buffer_Cleanup(&self->b);
	if (self->curl != NULL) {
		curl_easy_cleanup((CURL*)self->curl);
		self->curl = NULL;
	}
	if (self->authHeader != NULL) {
		curl_slist_free_all((struct curl_slist*)self->authHeader);
		self->authHeader = NULL;
	}
	if (self->token != NULL) {
		QBox_Token_Release(self->token);
		self->token = NULL;
	}
	if (self->root != NULL) {
		cJSON_Delete(self->root);
		self->root = NULL;
	}
}

static QBox_Error QBox_Client_initcall(QBox_Client* self, const char* url)
{
	QBox_Error err;
	CURL* curl = (CURL*)self->curl;

	curl_easy_reset(curl);
	QBox_Buffer_Reset(&self->b);
	if (self->root != NULL) {
		cJSON_Delete(self->root);
		self->root = NULL;
	}

    if (self->refreshToken) {
        if (self->expiry <= QBox_Seconds()) { // refresh
            err = QBox_Token_Refresh(self->token);
            if (err.code != 200) {
                return err;
            }
            curl_slist_free_all((struct curl_slist*)self->authHeader);
            QBox_Client_initAccess(self);
        }
    }

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, self->authHeader);

	err.code = 200;
	err.message = "OK";
	return err;
}

QBox_Error QBox_Client_CallWithBinary(
	QBox_Client* self, QBox_Json** ret, const char* url, FILE* body, QBox_Int64 bodyLen)
{
	CURL* curl;
	struct curl_slist* headers;
	QBox_Error err;

    if (self->vptr->QBox_Client_CallWithBinary) {
        err = self->vptr->QBox_Client_CallWithBinary(self, ret, url, body, bodyLen);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_initcall(self, url);
	if (err.code != 200) {
		return err;
	}
	curl = (CURL*)self->curl;
	curl_easy_setopt(curl, CURLOPT_INFILESIZE, bodyLen);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread);
	curl_easy_setopt(curl, CURLOPT_READDATA, body);
	curl_easy_setopt(curl, CURLOPT_POST, 1);

	char ctxLength[64];
	QBox_snprintf(ctxLength, 64, "Content-Length: %lld", bodyLen);
	headers = curl_slist_append(NULL, ctxLength);
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
	headers = curl_slist_append(headers, ((struct curl_slist*)self->authHeader)->data);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	err = QBox_callex(curl, &self->b, &self->root, QBox_False);

	curl_slist_free_all(headers);

	*ret = self->root;
	return err;
}

QBox_Error QBox_Client_Call(QBox_Client* self, QBox_Json** ret, const char* url)
{
	QBox_Error err;

    if (self->vptr->QBox_Client_Call) {
        err = self->vptr->QBox_Client_Call(self, ret, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_initcall(self, url);
	if (err.code != 200) {
		return err;
	}
	err = QBox_callex((CURL*)self->curl, &self->b, &self->root, QBox_False);
	*ret = self->root;
	return err;
}

QBox_Error QBox_Client_CallNoRet(QBox_Client* self, const char* url)
{
	QBox_Error err;

    if (self->vptr->QBox_Client_CallNoRet) {
        err = self->vptr->QBox_Client_CallNoRet(self, url);

        if (err.code != 200) {
            return err;
        }
    }

	err = QBox_Client_initcall(self, url);
	if (err.code != 200) {
		return err;
	}
	return QBox_callex((CURL*)self->curl, &self->b, &self->root, QBox_False);
}

/*============================================================================*/

static QBox_Error QBox_Client_Generate_AccessKey_Header(
    QBox_Client* self, char const* url, char const* addition, size_t addlen)
{
    QBox_Error err;
	struct curl_slist* headers = NULL;
    char const* path = NULL;
    char* auth = NULL;
    char digest[EVP_MAX_MD_SIZE + 1];
    unsigned int dgtlen = sizeof(digest);
    char* enc_digest = NULL;
    int ret = 0;
    HMAC_CTX ctx;

    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

	err.code    = 200;
	err.message = "OK";

    path = strstr(url, "://");

    if (path == NULL) {
        path = strstr(url, "/");
    }
    else {
        path = strstr(path + 3, "/");
    }

    /* Do digest calculation */
    HMAC_CTX_init(&ctx);

    ret = HMAC_Init_ex(&ctx, self->scrKey, strlen(self->scrKey), EVP_sha1(), NULL);

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    ret = HMAC_Update(&ctx, path, strlen(path));

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    ret = HMAC_Update(&ctx, "\n", 1);

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    if (addlen > 0) {
        ret = HMAC_Update(&ctx, addition, addlen);

        if (ret == 0) {
            HMAC_CTX_cleanup(&ctx);
            err.code    = 503;
            err.message = "Service Unavailable";
            return err;
        }
    }

    ret = HMAC_Final(&ctx, digest, &dgtlen);

    if (ret == 0) {
        HMAC_CTX_cleanup(&ctx);
        err.code    = 503;
        err.message = "Service Unavailable";
        return err;
    }

    HMAC_CTX_cleanup(&ctx);

    digest[dgtlen] = '\0';
    enc_digest = QBox_String_Encode(digest);

    /* Set appopriate HTTP header */
    auth = QBox_String_Concat("Authorization: QBox ", self->acsKey, ":", enc_digest, NULL);
    free(enc_digest);

    if (self->authHeader != NULL) {
	    curl_slist_free_all(self->authHeader);
        self->authHeader = NULL;
    }

	self->authHeader = curl_slist_append(NULL, auth);
    free(auth);

    return err;
}

static QBox_Error QBox_Client_CallWithBinary_ByAccessKey(
	QBox_Client* self, QBox_Json** ret, const char* url, FILE* body, QBox_Int64 bodyLen)
{
    return QBox_Client_Generate_AccessKey_Header(self, url, NULL, 0);
}

static QBox_Error QBox_Client_Call_ByAccessKey(QBox_Client* self, QBox_Json** ret, const char* url)
{
    return QBox_Client_Generate_AccessKey_Header(self, url, NULL, 0);
}

static QBox_Error QBox_Client_CallNoRet_ByAccessKey(QBox_Client* self, const char* url)
{
    return QBox_Client_Generate_AccessKey_Header(self, url, NULL, 0);
}

struct QBox_Virtual_Client_FuncTable QBox_Client_ByAccessKey_Func = {
    &QBox_Client_CallWithBinary_ByAccessKey,
    &QBox_Client_Call_ByAccessKey,
    &QBox_Client_CallNoRet_ByAccessKey
};

void QBox_Client_Init_ByAccessKey(QBox_Client* self, char const* accessKey, char const* secretKey, size_t bufSize)
{
	self->curl = curl_easy_init();
	self->root = NULL;
    self->refreshToken = 0;

    self->acsKey = accessKey;
    self->scrKey = secretKey;

    QBox_Buffer_Init(&self->b, bufSize);

    self->vptr = &QBox_Client_ByAccessKey_Func;
}
