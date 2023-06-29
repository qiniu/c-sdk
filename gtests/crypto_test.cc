#include <gtest/gtest.h>
#include "test.h"
#include "qiniu/private/crypto.h"

TEST(UnitTest, TestCryptoDigestMD5)
{
    Qiniu_Crypto_Init();
    Qiniu_Digest *digest = Qiniu_Digest_New(QINIU_DIGEST_TYPE_MD5);
    const uint8_t testData[] = {
        1, 2, 3, 4, 5
    };
    const uint8_t expectedMd5[] = {
        124, 253, 208, 120, 137,179, 41, 93 ,106 ,85 ,9 ,20 ,171, 53, 224 ,104
    };

    Qiniu_Digest_Update(digest, testData, sizeof(testData));

    uint8_t actualMd5[MD5_DIGEST_LENGTH];
    Qiniu_Digest_Final(digest, actualMd5, NULL);

    EXPECT_EQ(0, memcmp(expectedMd5, actualMd5, MD5_DIGEST_LENGTH));

    Qiniu_Digest_Free(digest);
    Qiniu_Crypto_Cleanup();
}

TEST(UnitTest, TestCryptoHmacSHA1)
{
    Qiniu_Crypto_Init();
    const uint8_t testKey[] = {
        1, 2, 3
    };
    Qiniu_HMAC *hmac = Qiniu_HMAC_New(QINIU_DIGEST_TYPE_SHA1, testKey, sizeof(testKey));
    EXPECT_TRUE(hmac != NULL);

    const uint8_t testData[] = {
        1, 2, 3, 4, 5
    };
    const uint8_t expectedSHA1[] = {
        179,87,136,100,211,195,50,46,56,190,5,22,106,230,199,54,161,15,168,178
    };

    EXPECT_EQ(QINIU_CRYPTO_RESULT_OK, Qiniu_HMAC_Update(hmac, testData, sizeof(testData)));

    uint8_t actualSHA1[SHA_DIGEST_LENGTH];

    EXPECT_EQ(QINIU_CRYPTO_RESULT_OK, Qiniu_HMAC_Final(hmac, actualSHA1, NULL));
    
    EXPECT_EQ(0, memcmp(expectedSHA1, actualSHA1, SHA_DIGEST_LENGTH));

    Qiniu_HMAC_Free(hmac);
    Qiniu_Crypto_Cleanup();
}