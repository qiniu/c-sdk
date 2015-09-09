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

/* @gist pfopargs */

typedef struct _Qiniu_FOP_PfopArgs {
	const char * bucket;
	const char * key;
	const char * notifyURL;
	int force;
	const char * pipeline;
} Qiniu_FOP_PfopArgs;

/* @endgist */

/* @gist pfopret */

typedef struct _Qiniu_FOP_PfopRet {
	const char*  persistentId;
} Qiniu_FOP_PfopRet;

/* @endgist */

Qiniu_Error Qiniu_FOP_Pfop(Qiniu_Client* self, Qiniu_FOP_PfopRet* ret, Qiniu_FOP_PfopArgs* args, char* fop[], int fopCount);

/*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* QINIU_FOP_H */
