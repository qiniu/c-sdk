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

#include "http.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*/
/* func Qiniu_FOP_Pfop */

/* @gist pfopret */

typedef struct _Qiniu_FOP_PfopRet {
    const char *persistentId;
} Qiniu_FOP_PfopRet;

/* @endgist */

QINIU_DLLAPI extern Qiniu_Error Qiniu_FOP_Pfop(Qiniu_Client *self, Qiniu_FOP_PfopRet *ret, const char *bucket,
                                               const char *key, char *fops[], int fopCount, const char *pipeline,
                                               const char *notifyURL, int force);

/*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* QINIU_FOP_H */
