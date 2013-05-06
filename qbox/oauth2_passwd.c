/*
 ============================================================================
 Name        : oauth2_passwd.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "oauth2_passwd.h"
#include <curl/curl.h>

static const char* QBOX_CLIENT_ID		= "a75604760c4da4caaa456c0c5895c061c3065c5a";
static const char* QBOX_CLIENT_SECRET	= "75df554a39f58accb7eb293b550fa59618674b7d";

#define QBox_initBufSize	1024

/*============================================================================*/
/* type QBox_call */

QBox_Error QBox_callex(CURL* curl, QBox_Buffer *resp, QBox_Json** ret, QBox_Bool simpleError, QBox_Buffer *resph);

static QBox_Error QBox_call(CURL* curl, int bufSize, QBox_Json** ret, QBox_Bool simpleError)
{
	QBox_Error err;
	QBox_Buffer resp;
	QBox_Buffer_Init(&resp, bufSize);

	err = QBox_callex(curl, &resp, ret, simpleError, NULL);

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

static void QBox_PasswordAuth_initAccess(QBox_PasswordAuth* self)
{
	char* accessToken;
	char* auth;

	accessToken = QBox_Token_Access(self->token, &self->expiry);
	auth = QBox_String_Concat2("Authorization: Bearer ", accessToken);
	free(accessToken);

	self->authHeader = curl_slist_append(NULL, auth);
	free(auth);
}

static QBox_Error QBox_PasswordAuth_Auth(void* self1, QBox_Header** header, const char* url, const char* addition, size_t addlen)
{
	QBox_Error err;
	QBox_PasswordAuth* self = (QBox_PasswordAuth*)self1;

	if (self->expiry <= QBox_Seconds()) { // refresh
		err = QBox_Token_Refresh(self->token);
		if (err.code != 200) {
			return err;
		}
		curl_slist_free_all((struct curl_slist*)self->authHeader);
		QBox_PasswordAuth_initAccess(self);
	}

	if (*header == NULL) {
		*header = self->authHeader;
	} else {
		*header = curl_slist_append(*header, self->authHeader->data);
	}

	err.code = 200;
	err.message = "OK";
	return err;
}

static void QBox_PasswordAuth_Release(void* self1)
{
	QBox_PasswordAuth* self = (QBox_PasswordAuth*)self1;

	if (self->authHeader != NULL) {
		curl_slist_free_all((struct curl_slist*)self->authHeader);
		self->authHeader = NULL;
	}
	if (self->token != NULL) {
		QBox_Token_Release(self->token);
		self->token = NULL;
	}
	free(self);
}

QBox_PasswordAuth* QBox_PasswordAuth_New(QBox_Token* token)
{
	QBox_PasswordAuth* self = (QBox_PasswordAuth*)malloc(sizeof(QBox_PasswordAuth));

	self->token = token;
	QBox_Token_Acquire(token);

	QBox_PasswordAuth_initAccess(self);
	return self;
}

/*============================================================================*/

static QBox_Auth_Itbl QBox_PasswordAuth_Itbl = {
	QBox_PasswordAuth_Auth,
	QBox_PasswordAuth_Release
};

void QBox_Client_InitByPassword(QBox_Client* self, QBox_Token* token, size_t bufSize)
{
	QBox_Auth auth = {QBox_PasswordAuth_New(token), &QBox_PasswordAuth_Itbl};
	QBox_Client_InitEx(self, auth, bufSize);
}

/*============================================================================*/

