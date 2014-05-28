/*
 ============================================================================
 Name        : mac_auth.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "http.h"
#include <curl/curl.h>
#include <openssl/hmac.h>
#include<openssl/engine.h>

#if defined(_WIN32)
#pragma comment(lib, "libeay32.lib")
#endif

/*============================================================================*/
/* Global */

void Qiniu_MacAuth_Init()
{
	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();
}

void Qiniu_MacAuth_Cleanup()
{
}

void Qiniu_Servend_Init(long flags)
{
	Qiniu_Global_Init(flags);
	Qiniu_MacAuth_Init();
}

void Qiniu_Servend_Cleanup()
{
	Qiniu_Global_Cleanup();
}

/*============================================================================*/
/* type Qiniu_Mac */

static Qiniu_Error Qiniu_Mac_Auth(
	void* self, Qiniu_Header** header, const char* url, const char* addition, size_t addlen)
{
	Qiniu_Error err;
	char* auth;
	char* enc_digest;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int dgtlen = sizeof(digest);
	HMAC_CTX ctx;
	Qiniu_Mac mac;

	char const* path = strstr(url, "://");
	if (path != NULL) {
		path = strchr(path + 3, '/');
	}
	if (path == NULL) {
		err.code = 400;
		err.message = "invalid url";
		return err;
	}

	if (self) {
		mac = *(Qiniu_Mac*)self;
	} else {
		mac.accessKey = QINIU_ACCESS_KEY;
		mac.secretKey = QINIU_SECRET_KEY;
	}

	HMAC_CTX_init(&ctx);
	HMAC_Init_ex(&ctx, mac.secretKey, strlen(mac.secretKey), EVP_sha1(), NULL);
	HMAC_Update(&ctx, path, strlen(path));
	HMAC_Update(&ctx, "\n", 1);

	if (addlen > 0) {
		HMAC_Update(&ctx, addition, addlen);
	}

	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

	enc_digest = Qiniu_Memory_Encode(digest, dgtlen);

	auth = Qiniu_String_Concat("Authorization: QBox ", mac.accessKey, ":", enc_digest, NULL);
	Qiniu_Free(enc_digest);

	*header = curl_slist_append(*header, auth);
	Qiniu_Free(auth);

	return Qiniu_OK;
}

static void Qiniu_Mac_Release(void* self)
{
	if (self) {
		free(self);
	}
}

static Qiniu_Mac* Qiniu_Mac_Clone(Qiniu_Mac* mac)
{
	Qiniu_Mac* p;
	char* accessKey;
	size_t n1, n2;
	if (mac) {
		n1 = strlen(mac->accessKey) + 1;
		n2 = strlen(mac->secretKey) + 1;
		p = (Qiniu_Mac*)malloc(sizeof(Qiniu_Mac) + n1 + n2);
		accessKey = (char*)(p + 1);
		memcpy(accessKey, mac->accessKey, n1);
		memcpy(accessKey + n1, mac->secretKey, n2);
		p->accessKey = accessKey;
		p->secretKey = accessKey + n1;
		return p;
	}
	return NULL;
}

static Qiniu_Auth_Itbl Qiniu_MacAuth_Itbl = {
	Qiniu_Mac_Auth,
	Qiniu_Mac_Release
};

Qiniu_Auth Qiniu_MacAuth(Qiniu_Mac* mac)
{
	Qiniu_Auth auth = {Qiniu_Mac_Clone(mac), &Qiniu_MacAuth_Itbl};
	return auth;
};

void Qiniu_Client_InitMacAuth(Qiniu_Client* self, size_t bufSize, Qiniu_Mac* mac)
{
	Qiniu_Auth auth = {Qiniu_Mac_Clone(mac), &Qiniu_MacAuth_Itbl};
	Qiniu_Client_InitEx(self, auth, bufSize);
}

/*============================================================================*/
/* func Qiniu_Mac_Sign*/

char* Qiniu_Mac_Sign(Qiniu_Mac* self, char* data)
{
	char* sign;
	char* encoded_digest;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int dgtlen = sizeof(digest);
	HMAC_CTX ctx;
	Qiniu_Mac mac;

	if (self) {
		mac = *self;
	} else {
		mac.accessKey = QINIU_ACCESS_KEY;
		mac.secretKey = QINIU_SECRET_KEY;
	}

	HMAC_CTX_init(&ctx);
	HMAC_Init_ex(&ctx, mac.secretKey, strlen(mac.secretKey), EVP_sha1(), NULL);
	HMAC_Update(&ctx, data, strlen(data));
	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

	encoded_digest = Qiniu_Memory_Encode(digest, dgtlen);
	sign = Qiniu_String_Concat3(mac.accessKey, ":", encoded_digest);
	Qiniu_Free(encoded_digest);

	return sign;
}

/*============================================================================*/
/* func Qiniu_Mac_SignToken */

char* Qiniu_Mac_SignToken(Qiniu_Mac* self, char* policy_str)
{
	char* data;
	char* sign;
	char* token;

	data = Qiniu_String_Encode(policy_str);
	sign = Qiniu_Mac_Sign(self, data);
	token = Qiniu_String_Concat3(sign, ":", data);

	Qiniu_Free(sign);
	Qiniu_Free(data);

	return token;
}

/*============================================================================*/

