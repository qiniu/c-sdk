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
} Qiniu_Io_form;

static Qiniu_Io_PutExtra qiniu_defaultExtra = { NULL, NULL, 0, 0 };

static void Qiniu_Io_form_init(
	Qiniu_Io_form* self, const char* uptoken, const char* key, Qiniu_Io_PutExtra* extra)
{
	Qiniu_Io_PutExtraParam* param;
	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;

	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "token", CURLFORM_COPYCONTENTS, uptoken, CURLFORM_END);

	if (extra == NULL) {
		extra = &qiniu_defaultExtra;
	}
	if (key != NULL) {
		curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "key", CURLFORM_COPYCONTENTS, key, CURLFORM_END);
	}
	for (param = extra->params; param != NULL; param = param->next) {
		curl_formadd(
			&formpost, &lastptr, CURLFORM_COPYNAME, param->key, CURLFORM_COPYCONTENTS, param->value, CURLFORM_END);
	}

	self->formpost = formpost;
	self->lastptr = lastptr;
}

/*============================================================================*/
/* func Qiniu_Io_PutXXX */

CURL* Qiniu_Client_reset(Qiniu_Client* self);
Qiniu_Error Qiniu_callex(CURL* curl, Qiniu_Buffer *resp, Qiniu_Json** ret, Qiniu_Bool simpleError, Qiniu_Buffer *resph);

static Qiniu_Error Qiniu_Io_call(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret, struct curl_httppost* formpost)
{
	Qiniu_Error err;

	CURL* curl = Qiniu_Client_reset(self);
	struct curl_slist* headers = curl_slist_append(NULL, "Expect:");

	curl_easy_setopt(curl, CURLOPT_URL, QINIU_UP_HOST);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	err = Qiniu_callex(curl, &self->b, &self->root, Qiniu_False, &self->respHeader);
	if (err.code == 200 && ret != NULL) {
		ret->hash = Qiniu_Json_GetString(self->root, "hash", NULL);
		ret->key = Qiniu_Json_GetString(self->root, "key", NULL);
	}

	curl_formfree(formpost);
	/*
	 * Bug No.(4718) Wang Xiaotao 2013\10\17 17:46:07
	 * Change for : free  variable 'headers'
	 * Reason     : memory leak!
	 */
	curl_slist_free_all(headers);
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

	return Qiniu_Io_call(self, ret, form.formpost);
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

	return Qiniu_Io_call(self, ret, form.formpost);
}

