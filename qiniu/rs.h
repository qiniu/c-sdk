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

#include "http.h"

/*============================================================================*/
/* type PutPolicy, GetPolicy */

/* @gist put-policy */

typedef struct _Qiniu_RS_PutPolicy {
    const char* scope;            // 必选项。可以是 bucketName 或者 bucketName:key
    const char* callbackUrl;      // 可选
    const char* callbackBody;     // 可选
    const char* returnUrl;        // 可选，更贴切的名字是 redirectUrl。
    const char* returnBody;       // 可选
    const char* endUser;          // 可选
    const char* asyncOps;         // 可选
    Qiniu_Uint32 expires;         // 可选。默认是 3600 秒
} Qiniu_RS_PutPolicy;

/* @endgist */

typedef struct _Qiniu_RS_GetPolicy {
    Qiniu_Uint32 expires;              // 可选。默认是 3600 秒
} Qiniu_RS_GetPolicy;

char* Qiniu_RS_PutPolicy_Token(Qiniu_RS_PutPolicy* policy, Qiniu_Mac* mac);
char* Qiniu_RS_GetPolicy_MakeRequest(Qiniu_RS_GetPolicy* policy, const char* baseUrl, Qiniu_Mac* mac);
char* Qiniu_RS_MakeBaseUrl(const char* domain, const char* key);

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

