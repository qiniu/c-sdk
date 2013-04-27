/*
 ============================================================================
 Name        : rs.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_RS_H
#define QINIU_RS_H

#include "oauth2.h"

/*============================================================================*/
/* type PutPolicy, GetPolicy */

typedef struct _Qiniu_RS_PutPolicy {
    const char* scope;                // 必选项。可以是 bucketName 或者 bucketName:key
    const char* callbackUrl;          // 可选
    const char* callbackBodyType;     // 可选
    const char* customer;             // 可选
    const char* asyncOps;             // 可选
    const char* returnBody;           // 可选
    Qiniu_Uint32 expires;              // 可选。默认是 3600 秒
    Qiniu_Uint16 escape;               // 可选。非 0 表示 Callback 的 Params 支持转义符
    Qiniu_Uint16 detectMime;           // 可选。非 0 表示在服务端自动检测文件内容的 MimeType
} Qiniu_RS_PutPolicy;

typedef struct _Qiniu_RS_GetPolicy {
    const char* scope;                // 格式是 domainPattern/keyPattern，没有默认值，用 */* 授权粒度过大，用 */key 比较合适。
    Qiniu_Uint32 expires;              // 可选。默认是 3600 秒
} Qiniu_RS_GetPolicy;

char* Qiniu_RS_PutPolicy_Token(Qiniu_RS_PutPolicy* policy);
char* Qiniu_RS_GetPolicy_Token(Qiniu_RS_GetPolicy* policy);

/*============================================================================*/
/* func Qiniu_RS_Stat */

/* @gist statret */

typedef struct _Qiniu_RS_StatRet {
	const char* hash;
	const char* mimeType;
	Qiniu_Int64 fsize;	
	Qiniu_Int64 putTime;
} Qiniu_RS_StatRet;

/* @endgist */

Qiniu_Error Qiniu_RS_Stat(
	Qiniu_Client* self, Qiniu_RS_StatRet* ret, const char* bucket, const char* key);

/*============================================================================*/
/* func Qiniu_RS_Delete */

Qiniu_Error Qiniu_RS_Delete(Qiniu_Client* self, const char* bucket, const char* key);

/*============================================================================*/

#endif /* QINIU_RS_H */

