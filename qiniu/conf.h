/*
 ============================================================================
 Name        : conf.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_CONF_H
#define QINIU_CONF_H

#include "macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

QINIU_DLLAPI extern const char* QINIU_ACCESS_KEY;
QINIU_DLLAPI extern const char* QINIU_SECRET_KEY;

QINIU_DLLAPI extern const char* QINIU_RS_HOST;
QINIU_DLLAPI extern const char* QINIU_UP_HOST;
QINIU_DLLAPI extern const char* QINIU_UC_HOST;
QINIU_DLLAPI extern const char* QINIU_API_HOST;
QINIU_DLLAPI extern const char* QINIU_FUSION_HOST;

#ifdef __cplusplus
}
#endif

#endif
