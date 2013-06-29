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

/*============================================================================*/
/* type Qiniu_RS_PutPolicy/GetPolicy */

char* Qiniu_RS_PutPolicy_Token(Qiniu_RS_PutPolicy* auth, Qiniu_Mac* mac)
{
	int expires;
	time_t deadline;
	char* authstr;
	char* token;

	cJSON* root = cJSON_CreateObject();

	if (auth->scope) {
		cJSON_AddStringToObject(root, "scope", auth->scope);
	}
	if (auth->callbackUrl) {
		cJSON_AddStringToObject(root, "callbackUrl", auth->callbackUrl);
	}
	if (auth->callbackBody) {
		cJSON_AddStringToObject(root, "callbackBody", auth->callbackBody);
	}
	if (auth->asyncOps) {
		cJSON_AddStringToObject(root, "asyncOps", auth->asyncOps);
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
	free(authstr);

	return token;
}

char* Qiniu_RS_GetPolicy_MakeRequest(Qiniu_RS_GetPolicy* auth, const char* baseUrl, Qiniu_Mac* mac)
{
	int expires;
	time_t deadline;
	char  e[11];
	char* authstr;
	char* token;
	char* request;

	if (auth->expires) {
		expires = auth->expires;
	} else {
		expires = 3600; // 1小时
	}
	time(&deadline);
	deadline += expires;
	sprintf(e, "%zu", (size_t)deadline);

	if (strchr(baseUrl, '?') != NULL) {
		authstr = Qiniu_String_Concat3(baseUrl, "&e=", e);
	} else {
		authstr = Qiniu_String_Concat3(baseUrl, "?e=", e);
	}

	token = Qiniu_Mac_Sign(mac, authstr);

	request = Qiniu_String_Concat3(authstr, "&token=", token);

	free(token);
	free(authstr);

	return request;
}

char* Qiniu_RS_MakeBaseUrl(const char* domain, const char* key)
{
	Qiniu_Bool fesc;
	char* baseUrl;
	char* escapedKey = Qiniu_PathEscape(key, &fesc);

	baseUrl = Qiniu_String_Concat("http://", domain, "/", escapedKey, NULL);

	if (fesc) {
		free(escapedKey);
	}

	return baseUrl;
}

/*============================================================================*/
/* func Qiniu_RS_Stat */

Qiniu_Error Qiniu_RS_Stat(
	Qiniu_Client* self, Qiniu_RS_StatRet* ret, const char* tableName, const char* key)
{
	Qiniu_Error err;
	cJSON* root;

	char* entryURI = Qiniu_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = Qiniu_String_Encode(entryURI);
	char* url = Qiniu_String_Concat3(QINIU_RS_HOST, "/stat/", entryURIEncoded);

	free(entryURI);
	free(entryURIEncoded);

	err = Qiniu_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->hash = Qiniu_Json_GetString(root, "hash", 0);
		ret->mimeType = Qiniu_Json_GetString(root, "mimeType", 0);
		ret->fsize = Qiniu_Json_GetInt64(root, "fsize", 0);
		ret->putTime = Qiniu_Json_GetInt64(root, "putTime", 0);
	}
	return err;
}

/*============================================================================*/
/* func Qiniu_RS_Delete */

Qiniu_Error Qiniu_RS_Delete(Qiniu_Client* self, const char* tableName, const char* key)
{
	Qiniu_Error err;

	char* entryURI = Qiniu_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = Qiniu_String_Encode(entryURI);
	char* url = Qiniu_String_Concat3(QINIU_RS_HOST, "/delete/", entryURIEncoded);

	free(entryURI);
	free(entryURIEncoded);

	err = Qiniu_Client_CallNoRet(self, url);
	free(url);

	return err;
}

/*============================================================================*/

