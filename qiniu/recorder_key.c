/*
 ============================================================================
 Name        : recorder_key.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "recorder_key.h"
/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

Qiniu_Recorder_Key_Generator Qiniu_Recorder_Key_Generator_New()
{
	Qiniu_Recorder_Key_Generator key;
	key.digest = Qiniu_Digest_New(QINIU_DIGEST_TYPE_MD5);
	return key;
}

void Qiniu_Recorder_Key_Generator_Append(Qiniu_Recorder_Key_Generator *key, const char *data)
{
	Qiniu_Digest_Update(key->digest, data, (int) strlen(data) + 1);
}

const char *Qiniu_Recorder_Key_Generator_Generate(Qiniu_Recorder_Key_Generator *key)
{
	unsigned char sign[MD5_DIGEST_LENGTH];
	int signLen = MD5_DIGEST_LENGTH * 2 + 1;
	char *signHex = (char *)malloc(sizeof(char) * signLen);
	char temp[3];

	Qiniu_Digest_Final(key->digest, sign, NULL);
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		Qiniu_snprintf(temp, 3, "%02x", sign[i]);
		temp[2] = '\0';
		memcpy(&(signHex[i * 2]), temp, 2);
	}
	signHex[signLen - 1] = '\0';
	return signHex;
}

void Qiniu_Recorder_Key_Generator_Free(Qiniu_Recorder_Key_Generator key) {
	Qiniu_Digest_Free(key.digest);
}

//---------------------------------------
