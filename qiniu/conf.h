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

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(USING_QINIU_LIBRARY_DLL)
    #define QINIU_DLLIMPORT __declspec(dllimport)
#else
    #define QINIU_DLLIMPORT 
#endif


QINIU_DLLIMPORT extern const char* QINIU_ACCESS_KEY;
QINIU_DLLIMPORT extern const char* QINIU_SECRET_KEY;

QINIU_DLLIMPORT extern const char* QINIU_RS_HOST;
QINIU_DLLIMPORT extern const char* QINIU_UP_HOST;

#ifdef __cplusplus
}
#endif

#endif
