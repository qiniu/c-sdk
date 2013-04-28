/*
 ============================================================================
 Name        : io.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "io.h"
#include <curl/curl.h>

/*============================================================================*/
/* func Qiniu_Io_form */

typedef struct _Qiniu_Io_form {
	struct curl_httppost* formpost;
	struct curl_httppost* lastptr;
	char* action;
} Qiniu_Io_form;

static void Qiniu_Io_form_init(
	Qiniu_Io_form* self, const char* uptoken, const char* key, Qiniu_Io_PutExtra* extra)
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
	Qiniu_Error err;

	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;

	if (mimeType == NULL) {
		mimeType = "application/octet-stream";
	}
	mimeTypeEncoded = Qiniu_String_Encode(mimeType);

	entryURI = Qiniu_String_Concat3(extra->bucket, ":", key);
	entryURIEncoded = Qiniu_String_Encode(entryURI);
	free(entryURI);

	action = Qiniu_String_Concat("/rs-put/", entryURIEncoded, "/mimeType/", mimeTypeEncoded, NULL);
	free(entryURIEncoded);
	free(mimeTypeEncoded);

	if (customMeta != NULL && *customMeta != '\0') {
		customMetaEncoded = Qiniu_String_Encode(customMeta);
		action2 = Qiniu_String_Concat3(action, "/meta/", customMetaEncoded);
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
/* func Qiniu_Io_PutXXX */

CURL* Qiniu_Client_reset(Qiniu_Client* self);
Qiniu_Error Qiniu_callex(CURL* curl, Qiniu_Buffer *resp, Qiniu_Json** ret, Qiniu_Bool simpleError, Qiniu_Buffer *resph);

static Qiniu_Error Qiniu_Io_call(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret, struct curl_httppost* formpost, char* action)
{
	Qiniu_Error err;

	CURL* curl = Qiniu_Client_reset(self);
	char* url = Qiniu_String_Concat2(QINIU_UP_HOST, "/upload");
	struct curl_slist* headers = curl_slist_append(NULL, "Expect:");

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
	if (err.code == 200 && ret != NULL) {
		ret->hash = Qiniu_Json_GetString(self->root, "hash", NULL);
	}

	curl_formfree(formpost);
	free(action);
	free(url);

	return err;
}

Qiniu_Error Qiniu_Io_PutFile(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, Qiniu_Io_PutExtra* extra)
{
	Qiniu_Io_form form;
	Qiniu_Io_form_init(&form, uptoken, key, extra);

	curl_formadd(
		&form.formpost, &form.lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, localFile, CURLFORM_END);

	return Qiniu_Io_call(self, ret, form.formpost, form.action);
}

Qiniu_Error Qiniu_Io_PutBuffer(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* buf, size_t fsize, Qiniu_Io_PutExtra* extra)
{
	Qiniu_Io_form form;
	Qiniu_Io_form_init(&form, uptoken, key, extra);

	curl_formadd(
		&form.formpost, &form.lastptr, CURLFORM_COPYNAME, "file",
		CURLFORM_BUFFER, key, CURLFORM_BUFFERPTR, buf, CURLFORM_BUFFERLENGTH, fsize, CURLFORM_END);

	return Qiniu_Io_call(self, ret, form.formpost, form.action);
}

