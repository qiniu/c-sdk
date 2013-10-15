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
	Qiniu_Free(authstr);

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
	sprintf(e, "%u", (unsigned int)deadline);

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

char* Qiniu_RS_MakeBaseUrl(const char* domain, const char* key)
{
	Qiniu_Bool fesc;
	char* baseUrl;
	char* escapedKey = Qiniu_PathEscape(key, &fesc);

	baseUrl = Qiniu_String_Concat("http://", domain, "/", escapedKey, NULL);

	if (fesc) {
		Qiniu_Free(escapedKey);
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

	Qiniu_Free(entryURI);
	Qiniu_Free(entryURIEncoded);

	err = Qiniu_Client_Call(self, &root, url);
	Qiniu_Free(url);

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

	Qiniu_Free(entryURI);
	Qiniu_Free(entryURIEncoded);

	err = Qiniu_Client_CallNoRet(self, url);
	Qiniu_Free(url);

	return err;
}

/*============================================================================*/
/* func Qiniu_RS_Copy */

Qiniu_Error Qiniu_RS_Copy(Qiniu_Client* self, 
	const char* tableNameSrc, const char* keySrc,
	const char* tableNameDest, const char* keyDest)
{
	Qiniu_Error err;

	char* entryURISrc = Qiniu_String_Concat3(tableNameSrc, ":", keySrc);
	char* entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
	char* entryURIDest = Qiniu_String_Concat3(tableNameDest, ":", keyDest);
	char* entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);
	char* urlPart = Qiniu_String_Concat3(entryURISrcEncoded, "/", entryURIDestEncoded);

	char* url = Qiniu_String_Concat3(QINIU_RS_HOST, "/copy/", urlPart);

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

Qiniu_Error Qiniu_RS_Move(Qiniu_Client* self, 
	const char* tableNameSrc, const char* keySrc,
	const char* tableNameDest, const char* keyDest)
{
	Qiniu_Error err;

	char* entryURISrc = Qiniu_String_Concat3(tableNameSrc, ":", keySrc);
	char* entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
	char* entryURIDest = Qiniu_String_Concat3(tableNameDest, ":", keyDest);
	char* entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);
	char* urlPart = Qiniu_String_Concat3(entryURISrcEncoded, "/", entryURIDestEncoded);
	char* url = Qiniu_String_Concat3(QINIU_RS_HOST, "/move/", urlPart);

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
/* func Qiniu_RS_BatchStat */

Qiniu_Error Qiniu_RS_BatchStat(
	Qiniu_Client* self, Qiniu_RS_BatchStatRet* rets,
	Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount)
{
	int code;
	Qiniu_Error err;
	cJSON *root, *arrayItem, *dataItem;
	char *body = NULL, *bodyTmp = NULL;
	char *entryURI, *entryURIEncoded, *opBody;
	Qiniu_RS_EntryPath* entry = entries;
	Qiniu_ItemCount curr = 0;
	Qiniu_ItemCount retSize = 0;
	char* url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

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
	/*
	 * Bug No.(4672) Wang Xiaotao  2013\10\15 17:56:00 
	 * Change for : free  var 'body'
	 * Reason     : memory leak!
	 */
	free(body);

	retSize = cJSON_GetArraySize(root);
	
	curr = 0;
	while (curr < retSize) {
		arrayItem = cJSON_GetArrayItem(root, curr);
		code = (int)Qiniu_Json_GetInt64(arrayItem, "code", 0);
		dataItem = cJSON_GetObjectItem(arrayItem, "data");

		rets[curr].code = code;

		if (code != 200) {
			rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
		} else {
			rets[curr].data.hash = Qiniu_Json_GetString(dataItem, "hash", 0);
			rets[curr].data.mimeType = Qiniu_Json_GetString(dataItem, "mimeType", 0);
			rets[curr].data.fsize = Qiniu_Json_GetInt64(dataItem, "fsize", 0);
			rets[curr].data.putTime = Qiniu_Json_GetInt64(dataItem, "putTime", 0);
		}
		curr++;
	}

	return err;
}

/*============================================================================*/
/* func Qiniu_RS_BatchDelete */

Qiniu_Error Qiniu_RS_BatchDelete(
	Qiniu_Client* self, Qiniu_RS_BatchItemRet* rets,
	Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount)
{
	int code;
	Qiniu_Error err;
	cJSON *root, *arrayItem, *dataItem;
	char *body = NULL, *bodyTmp = NULL;
	char *entryURI, *entryURIEncoded, *opBody;
	Qiniu_ItemCount curr = 0;
	Qiniu_ItemCount retSize = 0;
	Qiniu_RS_EntryPath* entry = entries;
	char* url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

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
	/*
	 * Bug No.(4672) Wang Xiaotao  2013\10\15 17:56:00 
	 * Change for : free  var 'body'
	 * Reason     : memory leak!
	 */
	free(body);

	retSize = cJSON_GetArraySize(root);

	curr = 0;
	while (curr < retSize) {
		arrayItem = cJSON_GetArrayItem(root, curr);
		code = (int)Qiniu_Json_GetInt64(arrayItem, "code", 0);
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
	Qiniu_Client* self, Qiniu_RS_BatchItemRet* rets,
	Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount)
{
	int code;
	Qiniu_Error err;
	cJSON *root, *arrayItem, *dataItem;
	char *body = NULL, *bodyTmp = NULL;
	char *entryURISrc, *entryURISrcEncoded, *opBody;
	char *entryURIDest, *entryURIDestEncoded, *bodyPart;
	Qiniu_ItemCount curr = 0;
	Qiniu_ItemCount retSize = 0;
	Qiniu_RS_EntryPathPair* entryPair = entryPairs;
	char* url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

	curr = 0;
	while (curr < entryCount) {
		entryURISrc = Qiniu_String_Concat3(entryPair->src.bucket, ":", entryPair->src.key);
		entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
		entryURIDest = Qiniu_String_Concat3(entryPair->dest.bucket, ":", entryPair->dest.key);
		entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);

		bodyPart = Qiniu_String_Concat3(entryURISrcEncoded, "/", entryURIDestEncoded);
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
	/*
	 * Bug No.(4672) Wang Xiaotao  2013\10\15 17:56:00 
	 * Change for : free  var 'body'
	 * Reason     : memory leak!
	 */
	free(body);

	retSize = cJSON_GetArraySize(root);
	
	curr = 0;
	while (curr < retSize) {
		arrayItem = cJSON_GetArrayItem(root, curr);
		code = (int)Qiniu_Json_GetInt64(arrayItem, "code", 0);
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
	Qiniu_Client* self, Qiniu_RS_BatchItemRet* rets,
	Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount)
{
	int code;
	Qiniu_Error err;
	cJSON *root, *arrayItem, *dataItem;
	char *body = NULL, *bodyTmp = NULL;
	char *entryURISrc, *entryURISrcEncoded, *opBody;
	char *entryURIDest, *entryURIDestEncoded, *bodyPart;
	Qiniu_ItemCount curr = 0;
	Qiniu_ItemCount retSize = 0;
	Qiniu_RS_EntryPathPair* entryPair = entryPairs;
	char* url = Qiniu_String_Concat2(QINIU_RS_HOST, "/batch");

	curr = 0;
	while (curr < entryCount) {
		entryURISrc = Qiniu_String_Concat3(entryPair->src.bucket, ":", entryPair->src.key);
		entryURISrcEncoded = Qiniu_String_Encode(entryURISrc);
		entryURIDest = Qiniu_String_Concat3(entryPair->dest.bucket, ":", entryPair->dest.key);
		entryURIDestEncoded = Qiniu_String_Encode(entryURIDest);
		
		bodyPart = Qiniu_String_Concat3(entryURISrcEncoded, "/", entryURIDestEncoded);
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
	/*
	 * Bug No.(4672) Wang Xiaotao  2013\10\15 17:56:00 
	 * Change for : free  var 'body'
	 * Reason     : memory leak!
	 */
	free(body);

	retSize = cJSON_GetArraySize(root);
	
	curr = 0;
	while (curr < retSize) {
		arrayItem = cJSON_GetArrayItem(root, curr);
		code = (int)Qiniu_Json_GetInt64(arrayItem, "code", 0);
		dataItem = cJSON_GetObjectItem(arrayItem, "data");

		rets[curr].code = code;

		if (code != 200) {
			rets[curr].error = Qiniu_Json_GetString(dataItem, "error", 0);
		}
		curr++;
	}

	return err;
}