/*
 ============================================================================
 Name        : tm.c
 Author      : Qiniu.com
 Copyright   : 2016(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "tm.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32

#include "emu_posix.h" // for type Qiniu_Posix_GetTimeOfDay

QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Tm_LocalTime(void)
{
	return Qiniu_Posix_GetTimeOfDay();
} // Qiniu

#else

#include <sys/time.h>


QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Tm_LocalTime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
} // Qiniu_Tm_LocalTime

#endif

#include <openssl/md5.h>
#include <string.h>

const char *Qiniu_MD5_HexStr(const char *src) {
	unsigned char *sign = (unsigned char *)calloc(sizeof(unsigned char), MD5_DIGEST_LENGTH);
	int signLen = MD5_DIGEST_LENGTH * 2 + 1;
	char *signHex = (char *)malloc(sizeof(char) * signLen);
	char temp[3];
	MD5_CTX md5Ctx;
	MD5_Init(&md5Ctx);
	MD5_Update(&md5Ctx, src, strlen(src));
	MD5_Final(sign, &md5Ctx);
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		Qiniu_snprintf(temp, 3, "%02x", sign[i]);
		temp[2] = '\0';
		memcpy(&(signHex[i * 2]), temp, 2);
	}
	signHex[signLen - 1] = '\0';
	Qiniu_Free(sign);
	return signHex;
} // Qiniu_MD5_HexStr

#ifdef __cplusplus
}
#endif

