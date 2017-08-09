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

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*/
/* type PutPolicy, GetPolicy */

/* @gist put-policy */

typedef struct _Qiniu_RS_PutPolicy {
    const char *scope;
    const char *saveKey;
    Qiniu_Uint32 isPrefixalScope;
    const char *callbackUrl;
    const char *callbackHost;
    const char *callbackBody;
    const char *callbackBodyType;
    const char *callbackFetchKey;
    const char *returnUrl;
    const char *returnBody;
    const char *endUser;
    const char *persistentOps;
    const char *persistentNotifyUrl;
    const char *persistentPipeline;
    const char *mimeLimit;
    Qiniu_Uint64 fsizeLimit;
    Qiniu_Uint64 fsizeMin;
    Qiniu_Uint32 detectMime;
    Qiniu_Uint32 insertOnly;
    Qiniu_Uint32 expires;
    Qiniu_Uint32 deleteAfterDays;
    Qiniu_Uint32 fileType;
} Qiniu_RS_PutPolicy;

/* @endgist */

typedef struct _Qiniu_RS_GetPolicy {
    Qiniu_Uint32 expires;
} Qiniu_RS_GetPolicy;

QINIU_DLLAPI extern char *Qiniu_RS_PutPolicy_Token(Qiniu_RS_PutPolicy *policy, Qiniu_Mac *mac);

QINIU_DLLAPI extern char *Qiniu_RS_GetPolicy_MakeRequest(Qiniu_RS_GetPolicy *policy, const char *baseUrl,
                                                         Qiniu_Mac *mac);

QINIU_DLLAPI extern char *Qiniu_RS_MakeBaseUrl(const char *domain, const char *key);

/*============================================================================*/
/* func Qiniu_RS_Stat */

/* @gist statret */

typedef struct _Qiniu_RS_StatRet {
    const char *hash;
    const char *mimeType;
    Qiniu_Int64 fsize;
    Qiniu_Int64 putTime;
    Qiniu_Int64 type;
} Qiniu_RS_StatRet;

/* @endgist */

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_Stat(Qiniu_Client *self, Qiniu_RS_StatRet *ret,
                                              const char *bucket, const char *key);

/*============================================================================*/
/* func Qiniu_RS_Delete */

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_Delete(Qiniu_Client *self, const char *bucket, const char *key);

/*============================================================================*/
/* func Qiniu_RS_Copy */

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_Copy(Qiniu_Client *self, const char *srcBucket, const char *srcKey,
                                              const char *destBucket, const char *destKey, Qiniu_Bool force);

/*============================================================================*/
/* func Qiniu_RS_Move */

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_Move(Qiniu_Client *self, const char *srcBucket, const char *srcKey,
                                              const char *destBucket, const char *destKey, Qiniu_Bool force);

/*============================================================================*/
/* func Qiniu_RS_ChangeMime */

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_ChangeMime(Qiniu_Client *self, const char *bucket, const char *key,
                                                    const char *newMime);
/*============================================================================*/
/*  func Qiniu_RS_ChangeType */
QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_ChangeType(Qiniu_Client *self, const char *bucket, const char *key,
                                                    const int fileType);

/*============================================================================*/
/* func Qiniu_RS_DeleteAfterDays */
QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_DeleteAfterDays(Qiniu_Client *self, const char *bucket, const char *key,
                                                         const int days);
/*============================================================================*/
/* @gist fetchret */

typedef struct _Qiniu_RS_FetchRet {
    const char *key;
    const char *hash;
    const char *mimeType;
    Qiniu_Int64 fsize;
} Qiniu_RS_FetchRet;

/* @endgist */

/* func Qiniu_RS_Fetch */
QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_Fetch(Qiniu_Client *self, Qiniu_RS_FetchRet *ret, const char *resURL,
                                               const char *bucket, const char *key);

/*============================================================================*/
/* func Qiniu_RS_Prefetch */
QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_Prefetch(Qiniu_Client *self, const char *bucket, const char *key);

/*============================================================================*/
/* func Qiniu_RS_BatchStat */

/* @gist entrypath */

typedef struct _Qiniu_RS_EntryPath {
    const char *bucket;
    const char *key;
} Qiniu_RS_EntryPath;

/* @endgist */

/* @gist batchstatret */

typedef struct _Qiniu_RS_BatchStatRet {
    Qiniu_RS_StatRet data;
    const char *error;
    int code;
} Qiniu_RS_BatchStatRet;

/* @endgist */

typedef int Qiniu_ItemCount;

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_BatchStat(Qiniu_Client *self, Qiniu_RS_BatchStatRet *rets,
                                                   Qiniu_RS_EntryPath *entries, Qiniu_ItemCount entryCount);

/*============================================================================*/
/* func Qiniu_RS_BatchDelete */

/* @gist batchitemret */

typedef struct _Qiniu_RS_BatchItemRet {
    const char *error;
    int code;
} Qiniu_RS_BatchItemRet;

/* @endgist */

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_BatchDelete(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                                     Qiniu_RS_EntryPath *entries, Qiniu_ItemCount entryCount);

/*============================================================================*/
/* func Qiniu_RS_BatchMove/Copy */

/* @gist entrypathpair */

typedef struct _Qiniu_RS_EntryPathPair {
    Qiniu_RS_EntryPath src;
    Qiniu_RS_EntryPath dest;
    Qiniu_Bool force;
} Qiniu_RS_EntryPathPair;

/* @endgist */

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_BatchMove(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                                   Qiniu_RS_EntryPathPair *entryPairs, Qiniu_ItemCount entryCount);

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_BatchCopy(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                                   Qiniu_RS_EntryPathPair *entryPairs, Qiniu_ItemCount entryCount);

/*============================================================================*/
/* func Qiniu_RS_BatchChgm/Chtype/DeleteAfterDays */

typedef struct _Qiniu_RS_EntryChangeType {
    const char *bucket;
    const char *key;
    int fileType;
} Qiniu_RS_EntryChangeType;

typedef struct _Qiniu_RS_EntryChangeMime {
    const char *bucket;
    const char *key;
    const char *mime;
} Qiniu_RS_EntryChangeMime;

typedef struct _Qiniu_RS_EntryDeleteAfterDays {
    const char *bucket;
    const char *key;
    int days;
} Qiniu_RS_EntryDeleteAfterDays;

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_BatchChangeType(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                                         Qiniu_RS_EntryChangeType *entry, Qiniu_ItemCount entryCount);

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_BatchChangeMime(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                                         Qiniu_RS_EntryChangeMime *entry, Qiniu_ItemCount entryCount);

QINIU_DLLAPI extern Qiniu_Error Qiniu_RS_BatchDeleteAfterDays(Qiniu_Client *self, Qiniu_RS_BatchItemRet *rets,
                                                              Qiniu_RS_EntryDeleteAfterDays *entry,
                                                              Qiniu_ItemCount entryCount);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* QINIU_RS_H */
