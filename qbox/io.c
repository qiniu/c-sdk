/*
 ============================================================================
 Name        : rscli.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : RS
 ============================================================================
 */

#include "io.h"
#include "conf.h"
#include <curl/curl.h>

/*============================================================================*/
/* func QBox_Io_form */

typedef struct _QBox_Io_form {
	struct curl_httppost* formpost;
	struct curl_httppost* lastptr;
	char* action;
} QBox_Io_form;

static void QBox_Io_form_init(
	QBox_Io_form* self, const char* uptoken, const char* key, QBox_Io_PutExtra* extra)
{
	const char* mimeType = extra->mimeType;
	const char* customMeta = extra->customMeta;
	const char* callbackParams = extra->callbackParams;

	char* mimeTypeEncoded;
	char* entryURI;
	char* entryURIEncoded;
	char* customMetaEncoded;
	char* action;
	char* action2;
	QBox_Error err;

	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;

	if (mimeType == NULL) {
		mimeType = "application/octet-stream";
	}
	mimeTypeEncoded = QBox_String_Encode(mimeType);

	entryURI = QBox_String_Concat3(extra->bucket, ":", key);
	entryURIEncoded = QBox_String_Encode(entryURI);
	free(entryURI);

	action = QBox_String_Concat("/rs-put/", entryURIEncoded, "/mimeType/", mimeTypeEncoded, NULL);
	free(entryURIEncoded);
	free(mimeTypeEncoded);

	if (customMeta != NULL && *customMeta != '\0') {
		customMetaEncoded = QBox_String_Encode(customMeta);
		action2 = QBox_String_Concat3(action, "/meta/", customMetaEncoded);
		free(action);
		free(customMetaEncoded);
		action = action2;
	}

	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "action", CURLFORM_COPYCONTENTS, action, CURLFORM_END);
	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "auth", CURLFORM_COPYCONTENTS, uptoken, CURLFORM_END);

	if (callbackParams != NULL && *callbackParams != '\0') {
		curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "params", CURLFORM_COPYCONTENTS, callbackParams, CURLFORM_END);
	}

	self->formpost = formpost;
	self->lastptr = lastptr;
	self->action = action;
}

/*============================================================================*/
/* func QBox_Io_PutXXX */

CURL* QBox_Client_reset(QBox_Client* self);
QBox_Error QBox_callex(CURL* curl, QBox_Buffer *resp, QBox_Json** ret, QBox_Bool simpleError);

QBox_Error QBox_Io_PutFile(
	QBox_Client* self, QBox_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, QBox_Io_PutExtra* extra)
{
	CURL* curl = QBox_Client_reset(self);
    char* url = QBox_String_Concat2(QBOX_UP_HOST, "/upload");
	QBox_Io_form form;
	QBox_Error err;

	QBox_Io_form_init(&form, uptoken, key, extra);

	curl_formadd(
		&form.formpost, &form.lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, localFile, CURLFORM_END);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, form.formpost);

	err = QBox_callex(curl, &self->b, &self->root, QBox_False);
	if (err.code == 200 && ret != NULL) {
		ret->hash = QBox_Json_GetString(self->root, "hash", NULL);
	}

	curl_formfree(form.formpost);
	free(form.action);
    free(url);

	return err;
}

QBox_Error QBox_Io_PutBuffer(
	QBox_Client* self, QBox_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* buf, size_t fsize, QBox_Io_PutExtra* extra)
{
	CURL* curl = QBox_Client_reset(self);
    char* url = QBox_String_Concat2(QBOX_UP_HOST, "/upload");
	QBox_Io_form form;
	QBox_Error err;

	QBox_Io_form_init(&form, uptoken, key, extra);

	curl_formadd(
		&form.formpost, &form.lastptr, CURLFORM_COPYNAME, "file",
		CURLFORM_BUFFER, "content", CURLFORM_BUFFERPTR, buf, CURLFORM_BUFFERLENGTH, fsize, CURLFORM_END);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, form.formpost);

	err = QBox_callex(curl, &self->b, &self->root, QBox_False);
	if (err.code == 200 && ret != NULL) {
		ret->hash = QBox_Json_GetString(self->root, "hash", NULL);
	}

	curl_formfree(form.formpost);
	free(form.action);
    free(url);

	return err;
}

