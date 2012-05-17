/*
 ============================================================================
 Name        : rscli.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : RS
 ============================================================================
 */

#include "rscli.h"
#include <curl/curl.h>

/*============================================================================*/
/* func QBox_RSCli_call */

static QBox_Error QBox_RSCli_call(CURL* curl, QBox_Buffer* resp)
{
	QBox_Error err;
	CURLcode curlCode;
	long httpCode;

	if (resp != NULL) {
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBox_Buffer_Fwrite);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
	} else {
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBox_Null_Fwrite);
	}
	curlCode = curl_easy_perform(curl);

	if (curlCode == 0) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		err.code = (int)httpCode;
		if (httpCode / 100 != 2) {
			err.message = "http status code is not OK";
		} else {
			err.message = "OK";
		}
	} else {
		err.code = curlCode;
		err.message = "curl_easy_perform error";
	}

	return err;
}

/*============================================================================*/
/* func QBox_RSCli_PutFile */

QBox_Error QBox_RSCli_PutFile(
	QBox_Buffer* resp, const char* url, const char* tableName, const char* key,
	const char* mimeType, const char* localFile, const char* customMeta, const char* callbackParams)
{
	char* entryURI;
	char* entryURIEncoded;
	char* mimeTypeEncoded;
	char* action;
	CURL* curl;
	QBox_Error err;

	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;

	if (mimeType == NULL) {
		mimeType = "application/octet-stream";
	}

	mimeTypeEncoded = QBox_String_Encode(mimeType);

	entryURI = QBox_String_Concat3(tableName, ":", key);
	entryURIEncoded = QBox_String_Encode(entryURI);
	free(entryURI);

	action = QBox_String_Concat("/rs-put/", entryURIEncoded, "/mimeType/", mimeTypeEncoded, NULL);
	free(entryURIEncoded);
	free(mimeTypeEncoded);

	curl = curl_easy_init();

	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "action", CURLFORM_COPYCONTENTS, action, CURLFORM_END);
	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, localFile, CURLFORM_END);
	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "params", CURLFORM_COPYCONTENTS, callbackParams, CURLFORM_END);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	err = QBox_RSCli_call(curl, resp);

	free(action);
	curl_formfree(formpost);
	return err;
}

/*============================================================================*/

