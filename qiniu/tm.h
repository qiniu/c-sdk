/*
 ============================================================================
 Name        : tm.h
 Author      : Qiniu.com
 Copyright   : 2016(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_TIME_H
#define QINIU_TIME_H

#include "base.h"

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Tm_LocalTime(void);

    QINIU_DLLAPI extern const char *Qiniu_MD5_HexStr(const char *src);
    QINIU_DLLAPI extern const char *Qiniu_MD5_HexStr_From_Reader(Qiniu_Reader r);

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif // QINIU_TIME_H
