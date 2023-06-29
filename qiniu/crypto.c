#include "private/crypto.h"
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <string.h>
#include "base.h"

#if OPENSSL_VERSION_NUMBER < 0x30000000
#include <openssl/engine.h>
#endif

void Qiniu_Crypto_Init() {
#if OPENSSL_VERSION_NUMBER < 0x30000000
	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();
#else
	OpenSSL_add_all_digests();
#endif
}

void Qiniu_Crypto_Cleanup() {
	// Noop
}

static const char *Qiniu_Digest_Type_To_Name(Qiniu_Digest_Type digest_type)
{
	switch (digest_type) {
	case QINIU_DIGEST_TYPE_SHA1:
		return "sha1";
	case QINIU_DIGEST_TYPE_MD5:
		return "md5";
	default:
		return NULL;
	}
}

/////// Digest implementation ///////
#if OPENSSL_VERSION_NUMBER < 0x30000000
struct Qiniu_Digest
{
	Qiniu_Digest_Type digest_type;
	void *ctx;
};

Qiniu_Digest* Qiniu_Digest_New(Qiniu_Digest_Type digest_type)
{
	Qiniu_Digest *self = (Qiniu_Digest*) malloc(sizeof(Qiniu_Digest));
	if (self == NULL) {
		return NULL;
	}
	memset(self, 0, sizeof(Qiniu_Digest));
	self->digest_type = digest_type;
	switch (digest_type)
	{
	case QINIU_DIGEST_TYPE_MD5:
		self->ctx = malloc(sizeof(MD5_CTX));
		if (self->ctx == NULL) {
			free(self);
			return NULL;
		}
		memset(self->ctx, 0, sizeof(MD5_CTX));
		if (MD5_Init((MD5_CTX*) self->ctx) == 0) {
			free(self->ctx);
			free(self);
			return NULL;
		}
		return self;
	case QINIU_DIGEST_TYPE_SHA1:
		self->ctx = malloc(sizeof(SHA_CTX));
		if (self->ctx == NULL) {
			free(self);
			return NULL;
		}
		memset(self->ctx, 0, sizeof(SHA_CTX));
		if (SHA1_Init((SHA_CTX*) self->ctx) == 0) {
			free(self->ctx);
			free(self);
			return NULL;
		}
		return self;
	default:
		return NULL;
	}
}

Qiniu_Crypto_Result Qiniu_Digest_Update(Qiniu_Digest *self, const void *data, int data_len)
{
	switch (self->digest_type)
	{
	case QINIU_DIGEST_TYPE_MD5:
		if (MD5_Update(self->ctx, data, data_len) == 0) {
			return QINIU_CRYPTO_RESULT_ERROR;
		}
		return QINIU_CRYPTO_RESULT_OK;
	case QINIU_DIGEST_TYPE_SHA1:
		if (SHA1_Update(self->ctx, data, data_len) == 0) {
			return QINIU_CRYPTO_RESULT_ERROR;
		}
		return QINIU_CRYPTO_RESULT_OK;
	default:
		return QINIU_CRYPTO_RESULT_ERROR;
	}
}

Qiniu_Crypto_Result Qiniu_Digest_Final(Qiniu_Digest *self, unsigned char *digest_output, size_t *digest_len_output)
{
	switch (self->digest_type) {
		case QINIU_DIGEST_TYPE_MD5:
		{
			unsigned char sign[MD5_DIGEST_LENGTH];
			if (MD5_Final(sign, (MD5_CTX*) self->ctx) == 0) {
				return QINIU_CRYPTO_RESULT_ERROR;
			}
			memcpy(digest_output, sign, MD5_DIGEST_LENGTH);
			if (digest_len_output != NULL) {
				*digest_len_output = MD5_DIGEST_LENGTH;
			}
			return QINIU_CRYPTO_RESULT_OK;
		}
		case QINIU_DIGEST_TYPE_SHA1:
		{
			unsigned char sign[SHA_DIGEST_LENGTH];
			if (SHA1_Final(sign, (SHA_CTX*) self->ctx) == 0) {
				return QINIU_CRYPTO_RESULT_ERROR;
			}
			memcpy(digest_output, sign, SHA_DIGEST_LENGTH);
			if (digest_len_output != NULL) {
				*digest_len_output = SHA_DIGEST_LENGTH;
			}
		}
		default:
			return QINIU_CRYPTO_RESULT_ERROR;
	}
}

void Qiniu_Digest_Free(Qiniu_Digest *self)
{
	free(self->ctx);
	free(self);
}
#else
// OpenSSL 3.0
struct Qiniu_Digest {
	EVP_MD_CTX *md_ctx;
};

Qiniu_Digest *Qiniu_Digest_New(Qiniu_Digest_Type digest_type)
{

	EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
	if (md_ctx == NULL) {
		return NULL;
	}
	const EVP_MD* md = EVP_get_digestbyname(Qiniu_Digest_Type_To_Name(digest_type));
	if (md == NULL) {
		EVP_MD_CTX_free(md_ctx);
		return NULL;
	}
	if (EVP_DigestInit_ex(md_ctx, md, NULL) == 0) {
		return NULL;
	}
	Qiniu_Digest *digest = malloc(sizeof(Qiniu_Digest));
	if (digest == NULL) {
		EVP_MD_CTX_free(md_ctx);
		return NULL;
	}
	memset(digest, 0, sizeof(Qiniu_Digest));
	digest->md_ctx = md_ctx;
	return digest;
}

Qiniu_Crypto_Result Qiniu_Digest_Update(Qiniu_Digest *self, const void *data, int data_len)
{
	if (EVP_DigestUpdate(self->md_ctx, data, data_len) == 0) {
		return QINIU_CRYPTO_RESULT_ERROR;
	}
	return QINIU_CRYPTO_RESULT_OK;
}

Qiniu_Crypto_Result Qiniu_Digest_Final(Qiniu_Digest *self, unsigned char *digest_output, size_t *digest_len_output)
{
	if (EVP_DigestFinal_ex(self->md_ctx, digest_output, (unsigned int *) digest_len_output) == 0) {
		return QINIU_CRYPTO_RESULT_ERROR;
	}
	return QINIU_CRYPTO_RESULT_OK;
}

void Qiniu_Digest_Free(Qiniu_Digest *self)
{
	EVP_MD_CTX_free(self->md_ctx);
	free(self);
}


#endif
/////// Digest end ///////

/////// HMAC implementation ///////
#if OPENSSL_VERSION_NUMBER < 0x30000000

struct Qiniu_HMAC {
	HMAC_CTX *md_ctx;
};

Qiniu_HMAC *Qiniu_HMAC_New(Qiniu_Digest_Type digest_type, const unsigned char *key, int key_len)
{
	Qiniu_HMAC *hmac = malloc(sizeof(Qiniu_HMAC));
	if (hmac == NULL) {
		goto error;
	}
	memset(hmac, 0, sizeof(Qiniu_HMAC));

#if OPENSSL_VERSION_NUMBER < 0x10100000
	hmac->md_ctx = (HMAC_CTX*) malloc(sizeof(HMAC_CTX));
	if (hmac->md_ctx == NULL) {
		goto error;
	}
	memset(hmac->md_ctx, 0, sizeof(HMAC_CTX));
	HMAC_CTX_init(hmac->md_ctx);
#else
	hmac->md_ctx = HMAC_CTX_new();
	if (hmac->md_ctx == NULL) {
		goto error;
	}
#endif
	const EVP_MD* md;
	switch (digest_type)
	{
	case QINIU_DIGEST_TYPE_SHA1:
		md = EVP_sha1();	
		break;
	case QINIU_DIGEST_TYPE_MD5:
		md = EVP_md5();
		break;
	default:
		goto error;
	}

	if (HMAC_Init_ex(hmac->md_ctx, key, key_len, md, NULL) == 0) {
		goto error;
	}
	return hmac;

error:
	if (hmac != NULL) {
		free(hmac);
	}
	return NULL;
}

Qiniu_Crypto_Result Qiniu_HMAC_Update(Qiniu_HMAC *self, const void *data, int data_len)
{
	if (HMAC_Update(self->md_ctx, data, data_len) == 0) {
		return QINIU_CRYPTO_RESULT_ERROR;
	}
	return QINIU_CRYPTO_RESULT_OK;
}

Qiniu_Crypto_Result Qiniu_HMAC_Final(Qiniu_HMAC *self, unsigned char *digest_output, size_t *digest_len_output)
{
	if (HMAC_Final(self->md_ctx, digest_output, (unsigned int *) digest_len_output) == 0) {
		return QINIU_CRYPTO_RESULT_ERROR;
	}
	return QINIU_CRYPTO_RESULT_OK;
}



void Qiniu_HMAC_Free(Qiniu_HMAC *self)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000
	HMAC_cleanup(self->md_ctx);
	free(self->md_ctx);
#else
	HMAC_CTX_free(self->md_ctx);
#endif
	free(self);
}
#else
// OpenSSL 3.0

struct Qiniu_HMAC {
	EVP_MD_CTX *md_ctx;
	EVP_PKEY *pkey;
};

Qiniu_HMAC *Qiniu_HMAC_New(Qiniu_Digest_Type digest_type, const unsigned char *key, int key_len)
{
	EVP_MD_CTX *md_ctx = EVP_MD_CTX_create();
	if (md_ctx == NULL) {
		return NULL;
	}
	if (EVP_MD_CTX_init(md_ctx) == 0) {
		EVP_MD_CTX_free(md_ctx);
		return NULL;
	}


	const EVP_MD* md = EVP_get_digestbyname(Qiniu_Digest_Type_To_Name(digest_type));
	if (md == NULL) {
		EVP_MD_CTX_free(md_ctx);
		return NULL;
	}
	EVP_PKEY *pkey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key, key_len);
	if (pkey == NULL) {
		EVP_MD_CTX_free(md_ctx);
		return NULL;
	}
	if (EVP_DigestSignInit(md_ctx, NULL, md, NULL, pkey) == 0) {
		EVP_MD_CTX_free(md_ctx);
		EVP_PKEY_free(pkey);
		return NULL;
	}
	Qiniu_HMAC *hmac = malloc(sizeof(Qiniu_HMAC));
	if (hmac == NULL) {
		EVP_MD_CTX_free(md_ctx);
		EVP_PKEY_free(pkey);
		return NULL;
	}
	hmac->md_ctx = md_ctx;
	hmac->pkey = pkey;
	return hmac;
}

Qiniu_Crypto_Result Qiniu_HMAC_Update(Qiniu_HMAC *self, const void *data, int data_len)
{
	if (EVP_DigestSignUpdate(self->md_ctx, data, data_len) == 0) {
		return QINIU_CRYPTO_RESULT_ERROR;
	}
	return QINIU_CRYPTO_RESULT_OK;
}

Qiniu_Crypto_Result Qiniu_HMAC_Final(Qiniu_HMAC *self, unsigned char *digest_output, size_t *digest_len_output)
{
	size_t tmp;
	if (EVP_DigestSignFinal(self->md_ctx, NULL, &tmp) == 0) {
		return QINIU_CRYPTO_RESULT_ERROR;
	}
	if (EVP_DigestSignFinal(self->md_ctx, digest_output, &tmp) == 0) {
		return QINIU_CRYPTO_RESULT_ERROR;
	}
	if (digest_len_output != NULL) {
		*digest_len_output = tmp;
	}
	return QINIU_CRYPTO_RESULT_OK;
}

void Qiniu_HMAC_Free(Qiniu_HMAC *self)
{
	EVP_PKEY_free(self->pkey);
	EVP_MD_CTX_destroy(self->md_ctx);
	free(self);
}
#endif
/////// HMAC end ///////
