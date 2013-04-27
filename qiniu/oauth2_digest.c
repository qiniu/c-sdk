/*
 ============================================================================
 Name        : oauth2_digest.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "oauth2.h"
#include <curl/curl.h>
#include <openssl/hmac.h>

/*============================================================================*/

static Qiniu_Error Qiniu_DigestAuth_Auth(
	void* self, Qiniu_Header** header, const char* url, const char* addition, size_t addlen)
{
	Qiniu_Error err;
	char const* path = NULL;
	char* auth = NULL;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int dgtlen = sizeof(digest);
	char* enc_digest = NULL;
	HMAC_CTX ctx;

	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();

	path = strstr(url, "://");
	if (path != NULL) {
		path = strchr(path + 3, '/');
	}
	if (path == NULL) {
		err.code = 400;
		err.message = "Invalid URL";
		return err;
	}

	/* Do digest calculation */
	HMAC_CTX_init(&ctx);

	HMAC_Init_ex(&ctx, QINIU_SECRET_KEY, strlen(QINIU_SECRET_KEY), EVP_sha1(), NULL);
	HMAC_Update(&ctx, path, strlen(path));
	HMAC_Update(&ctx, "\n", 1);

	if (addlen > 0) {
		HMAC_Update(&ctx, addition, addlen);
	}

	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

	enc_digest = Qiniu_Memory_Encode(digest, dgtlen);

	/* Set appopriate HTTP header */
	auth = Qiniu_String_Concat("Authorization: QBox ", QINIU_ACCESS_KEY, ":", enc_digest, NULL);
	free(enc_digest);

	*header = curl_slist_append(*header, auth);
	free(auth);

	err.code    = 200;
	err.message = "OK";
	return err;
}

static void Qiniu_DigestAuth_Release(void* self)
{
}

/*============================================================================*/

static Qiniu_Auth_Itbl Qiniu_DigestAuth_Itbl = {
	Qiniu_DigestAuth_Auth,
	Qiniu_DigestAuth_Release
};

static Qiniu_Auth Qiniu_DigestAuth = {
	NULL,
	&Qiniu_DigestAuth_Itbl
};

void Qiniu_Client_Init(Qiniu_Client* self, size_t bufSize)
{
	Qiniu_Client_InitEx(self, Qiniu_DigestAuth, bufSize);
}

/*============================================================================*/

