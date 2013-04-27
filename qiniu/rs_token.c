/*
 ============================================================================
 Name        : rs_token.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "rs.h"
#include "conf.h"
#include <stdlib.h>
#include <time.h>
#include <openssl/hmac.h>
#include "../cJSON/cJSON.h"

static char* Qiniu_makeToken(char* policy_str)
{
	char* token;
	char* encoded_digest;
	char* encoded_policy_str;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int dgtlen = sizeof(digest);

	HMAC_CTX ctx;

	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();

	encoded_policy_str = Qiniu_String_Encode(policy_str);
	free(policy_str);

	bzero(digest, sizeof(digest));

	HMAC_CTX_init(&ctx);
	HMAC_Init_ex(&ctx, QINIU_SECRET_KEY, strlen(QINIU_SECRET_KEY), EVP_sha1(), NULL);
	HMAC_Update(&ctx, encoded_policy_str, strlen(encoded_policy_str));
	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

	encoded_digest = Qiniu_Memory_Encode(digest, dgtlen);
	token = Qiniu_String_Concat(QINIU_ACCESS_KEY, ":", encoded_digest, ":", encoded_policy_str, NULL);
	free(encoded_policy_str);
	free(encoded_digest);

	return token;
}

char* Qiniu_RS_PutPolicy_Token(Qiniu_RS_PutPolicy* auth)
{
	int expires;
	time_t deadline;
	char* authstr;

	cJSON* root = cJSON_CreateObject();

	if (auth->scope) {
		cJSON_AddStringToObject(root, "scope", auth->scope);
	}
	if (auth->callbackUrl) {
		cJSON_AddStringToObject(root, "callbackUrl", auth->callbackUrl);
	}
	if (auth->callbackBodyType) {
		cJSON_AddStringToObject(root, "callbackBodyType", auth->callbackBodyType);
	}
	if (auth->asyncOps) {
		cJSON_AddStringToObject(root, "asyncOps", auth->asyncOps);
	}
	if (auth->returnBody) {
		cJSON_AddStringToObject(root, "returnBody", auth->returnBody);
	}
	if (auth->customer) {
		cJSON_AddStringToObject(root, "customer", auth->customer);
	}

	if (auth->expires) {
		expires = auth->expires;
	} else {
		expires = 3600; // 1小时
	}
	time(&deadline);
	deadline += expires;
	cJSON_AddNumberToObject(root, "deadline", deadline);

	if (auth->escape) {
		cJSON_AddNumberToObject(root, "escape", auth->escape);
	}
	if (auth->detectMime) {
		cJSON_AddNumberToObject(root, "detectMime", auth->detectMime);
	}

	authstr = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

	return Qiniu_makeToken(authstr);
}

char* Qiniu_RS_GetPolicy_Token(Qiniu_RS_GetPolicy* auth)
{
	int expires;
	time_t deadline;
	char* authstr;

	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "S", auth->scope);

	if (auth->expires) {
		expires = auth->expires;
	} else {
		expires = 3600; // 1小时
	}
	time(&deadline);
	deadline += expires;
	cJSON_AddNumberToObject(root, "E", deadline);

	authstr = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

	return Qiniu_makeToken(authstr);
}

/*============================================================================*/

