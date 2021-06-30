/*
 ============================================================================
 Name        : recorder_key.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_RECORDER_KEY_H
#define QINIU_RECORDER_KEY_H

#include "base.h"
#include <openssl/md5.h>

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

    /*============================================================================*/

    struct Qiniu_Recorder_Key_Generator;

    typedef struct Qiniu_Recorder_Key_Generator
    {
        MD5_CTX md5Ctx;
    } Qiniu_Recorder_Key_Generator;

    QINIU_DLLAPI extern Qiniu_Recorder_Key_Generator Qiniu_Recorder_Key_Generator_New();
    QINIU_DLLAPI extern void Qiniu_Recorder_Key_Generator_Append(Qiniu_Recorder_Key_Generator *key, const char *data);
    QINIU_DLLAPI extern const char *Qiniu_Recorder_Key_Generator_Generate(Qiniu_Recorder_Key_Generator *key);

    /*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // QINIU_RECORDER_KEY_H
