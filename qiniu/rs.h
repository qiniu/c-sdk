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

#pragma pack(1)

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
/* func Qiniu_RS_Copy */

Qiniu_Error Qiniu_RS_Copy(Qiniu_Client* self, 
        const char* tableNameSrc, const char* keySrc, 
        const char* tableNameDest, const char* keyDest);

/*============================================================================*/
/* func Qiniu_RS_Move */

Qiniu_Error Qiniu_RS_Move(Qiniu_Client* self, 
        const char* tableNameSrc, const char* keySrc, 
        const char* tableNameDest, const char* keyDest);

/*============================================================================*/
/* func Qiniu_RS_BatchStat */

/* @gist entrypath */

typedef struct _Qiniu_RS_EntryPath {
    const char* bucket;
    const char* key;
} Qiniu_RS_EntryPath;

/* @endgist */

/* @gist batchstatret */

typedef struct _Qiniu_RS_BatchStatRet {
    Qiniu_RS_StatRet data;
    const char* error;
    int code;
}Qiniu_RS_BatchStatRet;

/* @endgist */

typedef int Qiniu_ItemCount;

Qiniu_Error Qiniu_RS_BatchStat(
        Qiniu_Client* self, Qiniu_RS_BatchStatRet* rets,
        Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount);

/*============================================================================*/
/* func Qiniu_RS_BatchDelete */

/* @gist batchitemret */

typedef struct _Qiniu_RS_BatchItemRet {
    const char* error;
    int code;
}Qiniu_RS_BatchItemRet;

/* @endgist */

Qiniu_Error Qiniu_RS_BatchDelete(
        Qiniu_Client* self, Qiniu_RS_BatchItemRet* rets,
        Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount);

/*============================================================================*/
/* func Qiniu_RS_BatchMove/Copy */

/* @gist entrypathpair */

typedef struct _Qiniu_RS_EntryPathPair {
    Qiniu_RS_EntryPath src;
    Qiniu_RS_EntryPath dest;
} Qiniu_RS_EntryPathPair;

/* @endgist */

Qiniu_Error Qiniu_RS_BatchMove(
        Qiniu_Client* self, Qiniu_RS_BatchItemRet* rets,
        Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount);

Qiniu_Error Qiniu_RS_BatchCopy(
        Qiniu_Client* self, Qiniu_RS_BatchItemRet* rets,
        Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount);

/*============================================================================*/

#pragma pack()

#endif /* QINIU_RS_H */
