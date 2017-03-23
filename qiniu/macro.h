/*
 ============================================================================
 Name        : conf.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_MACRO_H
#define QINIU_MACRO_H

#if defined(USING_QINIU_LIBRARY_DLL)
    #define QINIU_DLLAPI __declspec(dllimport)
#elif defined(COMPILING_QINIU_LIBRARY_DLL)
    #define QINIU_DLLAPI __declspec(dllexport)
#else
    #define QINIU_DLLAPI 
#endif

#endif
