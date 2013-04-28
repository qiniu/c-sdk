/*
 ============================================================================
 Name        : rs.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */
#include "rs.h"

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

