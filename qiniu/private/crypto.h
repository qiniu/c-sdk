#ifndef QINIU_CRYPTO_H
#define QINIU_CRYPTO_H
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

void Qiniu_Crypto_Init();
void Qiniu_Crypto_Cleanup();

typedef enum Qiniu_Crypto_Result {
	QINIU_CRYPTO_RESULT_OK = 0,
	QINIU_CRYPTO_RESULT_ERROR = 1,
} Qiniu_Crypto_Result;

typedef enum Qiniu_Digest_Type {
	QINIU_DIGEST_TYPE_SHA1 = 0,
	QINIU_DIGEST_TYPE_MD5 = 1,
} Qiniu_Digest_Type;

#define SHA_DIGEST_LENGTH 20
#define MD5_DIGEST_LENGTH 16
#define EVP_MAX_MD_SIZE 64

typedef struct Qiniu_Digest Qiniu_Digest;
Qiniu_Digest* Qiniu_Digest_New(Qiniu_Digest_Type digest_type);
Qiniu_Crypto_Result Qiniu_Digest_Update(Qiniu_Digest *self, const void *data, int data_len);
Qiniu_Crypto_Result Qiniu_Digest_Final(Qiniu_Digest *self, unsigned char *digest_output, size_t *digest_len_output);
void Qiniu_Digest_Free(Qiniu_Digest *self);

typedef struct Qiniu_HMAC Qiniu_HMAC;
Qiniu_HMAC* Qiniu_HMAC_New(Qiniu_Digest_Type digest_type, const unsigned char *key, int key_len);
Qiniu_Crypto_Result Qiniu_HMAC_Update(Qiniu_HMAC *self, const void *data, int data_len);
Qiniu_Crypto_Result Qiniu_HMAC_Final(Qiniu_HMAC *self, unsigned char *digest_output, size_t *digest_len_output);
void Qiniu_HMAC_Free(Qiniu_HMAC *self);

#ifdef __cplusplus
}
#endif

#endif // QINIU_CRYPTO_H