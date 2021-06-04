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
    MD5_Init(&key.md5Ctx);
    return key;
}

void Qiniu_Recorder_Key_Generator_Append(Qiniu_Recorder_Key_Generator *key, const char *data)
{
    MD5_Update(&key->md5Ctx, data, strlen(data) + 1);
}

const char *Qiniu_Recorder_Key_Generator_Generate(Qiniu_Recorder_Key_Generator *key)
{
    unsigned char sign[MD5_DIGEST_LENGTH];
    int signLen = MD5_DIGEST_LENGTH * 2 + 1;
    char *signHex = (char *)malloc(sizeof(char) * signLen);
    char temp[3];

    MD5_Final(sign, &key->md5Ctx);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        Qiniu_snprintf(temp, 3, "%02x", sign[i]);
        temp[2] = '\0';
        memcpy(&(signHex[i * 2]), temp, 2);
    }
    signHex[signLen - 1] = '\0';
    return signHex;
}

//---------------------------------------
