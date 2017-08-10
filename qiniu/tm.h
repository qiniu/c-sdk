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

#ifdef __cplusplus
extern "C"
{
#endif

QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Tm_LocalTime(void);

QINIU_DLLAPI extern const char *Qiniu_MD5_HexStr(const char *src);

#ifdef __cplusplus
}
#endif

#endif // QINIU_TIME_H
