/*
 ============================================================================
 Name        : fop.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include <curl/curl.h>

#include "fop.h"
#include "../cJSON/cJSON.h"

Qiniu_Error Qiniu_FOP_Pfop(
	Qiniu_Client* self,
	Qiniu_FOP_PfopRet* ret,
	Qiniu_FOP_PfopArgs* args,
	char* fop[],
	int fopCount)
{
	Qiniu_Error err;
	cJSON* root = NULL;
	char* fops = NULL;
	char* encodedBucket = NULL;
	char* encodedKey = NULL;
	char* encodedFops = NULL;
	char* encodedNotifyURL = NULL;
	char* url = NULL;
	char* body = NULL;
	char* items[6];
	int itemCount = 0;
	int i = 0;

	// Add encoded bucket
	encodedBucket = curl_easy_escape(self->curl, args->bucket, strlen(args->bucket));
	if (encodedBucket == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		return err;
	}

	items[itemCount] = Qiniu_String_Concat2("bucket=", encodedBucket);
	curl_free(encodedBucket);
	if (items[itemCount] == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		return err;
	}
	itemCount += 1;

	// Add encoded key
	encodedKey = curl_easy_escape(self->curl, args->key, strlen(args->key));
	if (encodedKey == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		goto PFOP_ERROR_HANDLING;
	}

	items[itemCount] = Qiniu_String_Concat2("key=", encodedKey);
	curl_free(encodedKey);
	if (items[itemCount] == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		goto PFOP_ERROR_HANDLING;
	}
	itemCount += 1;

	// Add encoded fops
	fops = Qiniu_String_Join(";", fop, fopCount);
	if (fops == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		goto PFOP_ERROR_HANDLING;
	}

	encodedFops = curl_easy_escape(self->curl, fops, strlen(fops));
	Qiniu_Free(fops);
	if (encodedFops == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		goto PFOP_ERROR_HANDLING;
	}
	
	items[itemCount] = Qiniu_String_Concat2("fops=", encodedFops);
	curl_free(encodedFops);
	if (items[itemCount] == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		goto PFOP_ERROR_HANDLING;
	}
	itemCount += 1;

	// Add encoded notifyURL
	if (args->notifyURL) {
		encodedNotifyURL = curl_easy_escape(self->curl, args->notifyURL, strlen(args->notifyURL));
		if (encodedNotifyURL == NULL) {
			err.code = 499;
			err.message = "No enough memory";
			goto PFOP_ERROR_HANDLING;
		}

		items[itemCount] = Qiniu_String_Concat2("notifyURL=", encodedNotifyURL);
		curl_free(encodedNotifyURL);
		if (items[itemCount] == NULL) {
			err.code = 499;
			err.message = "No enough memory";
			goto PFOP_ERROR_HANDLING;
		}
		itemCount += 1;
	} // if

	// Add force
	if (args->force == 1) {
		items[itemCount] = Qiniu_String_Dup("force=1");
		if (items[itemCount] == NULL) {
			err.code = 499;
			err.message = "No enough memory";
			goto PFOP_ERROR_HANDLING;
		}
		itemCount += 1;
	} // if

	// Add pipeline
	if (args->pipeline) {
		items[itemCount] = Qiniu_String_Concat2("pipeline=", args->pipeline);
		if (items[itemCount] == NULL) {
			err.code = 499;
			err.message = "No enough memory";
			goto PFOP_ERROR_HANDLING;
		}
		itemCount += 1;
	} // if

	body = Qiniu_String_Join("&", items, itemCount);
	if (body == NULL) {
		err.code = 499;
		err.message = "No enough memory";
		goto PFOP_ERROR_HANDLING;
	} // if

	url = Qiniu_String_Concat2(QINIU_API_HOST, "/pfop");
	if (url == NULL) {
		Qiniu_Free(body);
		err.code = 499;
		err.message = "No enough memory";
		goto PFOP_ERROR_HANDLING;
	} // if

	err = Qiniu_Client_CallWithBuffer(
		self,
		&root,
		url,
		body,
		strlen(body),
		"application/x-www-form-urlencoded"
	);
	Qiniu_Free(url);
	Qiniu_Free(body);
	if (err.code == 200) {
		ret->persistentId = Qiniu_Json_GetString(root, "persistentId", 0);
	}

PFOP_ERROR_HANDLING:
	for (i = itemCount - 1; i >= 0; i -= 1) {
		Qiniu_Free(items[i]);
	} // for
	return err;
} // Qiniu_FOP_Pfop
