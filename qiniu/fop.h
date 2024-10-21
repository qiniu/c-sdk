/*
 ============================================================================
 Name        : fop.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_FOP_H
#define QINIU_FOP_H

#include "rs.h"
#include "rfc3339.h"

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*/
/* func Qiniu_FOP_Pfop */

/* @gist pfop */

typedef struct _Qiniu_FOP_PfopParams
{
    const char *bucket;
    const char *key;
    const char *pipeline;
    const char *notifyURL;
    const char *workflowTemplateID;
    char **fops;
    int fopCount;
    int force;
    int type;
} Qiniu_FOP_PfopParams;

/* @endgist */

/* @gist pfopret */

typedef struct _Qiniu_FOP_PfopRet {
    const char *persistentId;
} Qiniu_FOP_PfopRet;

/* @endgist */

/* @gist prefopret */

typedef struct _Qiniu_FOP_PrefopRet
{
    const char *id;
    const char *desc;
    const char *inputBucket;
    const char *inputKey;
    const char *taskFrom;
    int code;
    int type;
    Qiniu_DateTime creationDate;
} Qiniu_FOP_PrefopRet;

typedef struct _Qiniu_FOP_PrefopItemRet
{
    const char *cmd;
    const char *desc;
    const char *error;
    const char *hash;
    const char *key;
    int code;
    int returnOld;
} Qiniu_FOP_PrefopItemRet;

/* @endgist */

QINIU_DLLAPI extern Qiniu_Error
Qiniu_FOP_Pfop(Qiniu_Client *self, Qiniu_FOP_PfopRet *ret, const char *bucket,
               const char *key, char *fops[], int fopCount, const char *pipeline,
               const char *notifyURL, int force);
QINIU_DLLAPI extern Qiniu_Error Qiniu_FOP_Pfop_v2(Qiniu_Client *self, Qiniu_FOP_PfopRet *ret, Qiniu_FOP_PfopParams *params);
QINIU_DLLAPI extern Qiniu_Error
Qiniu_FOP_Prefop(Qiniu_Client *self, Qiniu_FOP_PrefopRet *ret, Qiniu_FOP_PrefopItemRet *itemsRet,
                 Qiniu_ItemCount *itemsCount, const char *persistentId, Qiniu_ItemCount maxItemsCount);

/*============================================================================*/

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif /* QINIU_FOP_H */
