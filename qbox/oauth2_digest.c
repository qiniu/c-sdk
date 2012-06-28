/*
 ============================================================================
 Name        : oauth2_digest.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "oauth2.h"
#include <curl/curl.h>
#include <openssl/hmac.h>

/*============================================================================*/

static QBox_Error QBox_DigestAuth_Auth(void* self, QBox_Header** header, const char* url, const char* addition, size_t addlen)
{
	QBox_Error err;
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
		err.message = "invalid url";
		return err;
	}

	/* Do digest calculation */
	HMAC_CTX_init(&ctx);

	HMAC_Init_ex(&ctx, QBOX_SECRET_KEY, strlen(QBOX_SECRET_KEY), EVP_sha1(), NULL);
	HMAC_Update(&ctx, path, strlen(path));
	HMAC_Update(&ctx, "\n", 1);

	if (addlen > 0) {
		HMAC_Update(&ctx, addition, addlen);
	}

	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

	digest[dgtlen] = '\0';
	enc_digest = QBox_String_Encode(digest);

	/* Set appopriate HTTP header */
	auth = QBox_String_Concat("Authorization: QBox ", QBOX_ACCESS_KEY, ":", enc_digest, NULL);
	free(enc_digest);

	*header = curl_slist_append(*header, auth);
	free(auth);

	err.code    = 200;
	err.message = "OK";
	return err;
}

static void QBox_DigestAuth_Release(void* self)
{
}

/*============================================================================*/

static QBox_Auth_Vtable QBox_DigestAuth_Vtable = {
	QBox_DigestAuth_Auth,
	QBox_DigestAuth_Release
};

void QBox_Client_Init(QBox_Client* self, size_t bufSize)
{
	QBox_Client_InitEx(self, NULL, &QBox_DigestAuth_Vtable, bufSize);
}

/*============================================================================*/

