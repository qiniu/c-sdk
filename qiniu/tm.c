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

QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Tm_LocalTime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec;
} // Qiniu_Tm_LocalTime

#endif

#include "private/crypto.h"
#include <string.h>

	const char *Qiniu_MD5_HexStr(const char *src)
	{
		unsigned char sign[MD5_DIGEST_LENGTH];
		int signLen = MD5_DIGEST_LENGTH * 2 + 1;
		char *signHex = (char *)malloc(sizeof(char) * signLen);
		char temp[3];
		Qiniu_Digest *md5_digest = Qiniu_Digest_New(QINIU_DIGEST_TYPE_MD5);
		Qiniu_Digest_Update(md5_digest, src, (int)strlen(src));
		Qiniu_Digest_Final(md5_digest, sign, NULL);
		Qiniu_Digest_Free(md5_digest);
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
		{
			Qiniu_snprintf(temp, 3, "%02x", sign[i]);
			temp[2] = '\0';
			memcpy(&(signHex[i * 2]), temp, 2);
		}
		signHex[signLen - 1] = '\0';
		return signHex;
	} // Qiniu_MD5_HexStr

	const char *Qiniu_MD5_HexStr_From_Reader(Qiniu_Reader r)
	{

		unsigned char *sign = (unsigned char *)calloc(sizeof(unsigned char), MD5_DIGEST_LENGTH);
		int signLen = MD5_DIGEST_LENGTH * 2 + 1;
		char *signHex = (char *)malloc(sizeof(char) * signLen);
		char temp[3];
		Qiniu_Digest *md5_digest = Qiniu_Digest_New(QINIU_DIGEST_TYPE_MD5);

		{
			size_t unused = 1;
			size_t buffLen = (4 << 20); //read 4M each time
			char *buff = (char *)malloc(sizeof(char) * buffLen);
			size_t n = 0;
			do
			{
				n = r.Read(buff, unused, buffLen, r.self);
				Qiniu_Digest_Update(md5_digest, buff, (int)n);
			} while (n == buffLen);
			free(buff);
		}
		Qiniu_Digest_Final(md5_digest, sign, NULL);
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
		{
			Qiniu_snprintf(temp, 3, "%02x", sign[i]);
			temp[2] = '\0';
			memcpy(&(signHex[i * 2]), temp, 2);
		}
		signHex[signLen - 1] = '\0';
		Qiniu_Free(sign);
		return signHex;
	}

#ifdef __cplusplus
}
#endif
