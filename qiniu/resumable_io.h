/*
 ============================================================================
 Name        : resumable_io.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_RESUMABLE_IO_H
#define QINIU_RESUMABLE_IO_H

#include "http.h"
#include "io.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*/

#define Qiniu_Rio_InvalidCtx			701
#define Qiniu_Rio_UnmatchedChecksum		9900
#define Qiniu_Rio_InvalidPutProgress	9901
#define Qiniu_Rio_PutFailed				9902
#define Qiniu_Rio_PutInterrupted		9903

/*============================================================================*/
/* type Qiniu_Rio_WaitGroup */

typedef struct _Qiniu_Rio_WaitGroup_Itbl {
	void (*Add)(void* self, int n);
	void (*Done)(void* self);
	void (*Wait)(void* self);
	void (*Release)(void* self);
} Qiniu_Rio_WaitGroup_Itbl;

typedef struct _Qiniu_Rio_WaitGroup {
	void* self;
	Qiniu_Rio_WaitGroup_Itbl* itbl;
} Qiniu_Rio_WaitGroup;

#if defined(_WIN32)

Qiniu_Rio_WaitGroup Qiniu_Rio_MTWG_Create(void);

#endif

/*============================================================================*/
/* type Qiniu_Rio_ThreadModel */

typedef struct _Qiniu_Rio_ThreadModel_Itbl {
	Qiniu_Rio_WaitGroup (*WaitGroup)(void* self);
	Qiniu_Client* (*ClientTls)(void* self, Qiniu_Client* mc);
	int (*RunTask)(void* self, void (*task)(void* params), void* params);
} Qiniu_Rio_ThreadModel_Itbl;

typedef struct _Qiniu_Rio_ThreadModel {
	void* self;
	Qiniu_Rio_ThreadModel_Itbl* itbl;
} Qiniu_Rio_ThreadModel;

QINIU_DLLAPI extern Qiniu_Rio_ThreadModel Qiniu_Rio_ST;

/*============================================================================*/
/* type Qiniu_Rio_Settings */

typedef struct _Qiniu_Rio_Settings {
	int taskQsize;
	int workers;
	int chunkSize;
	int tryTimes;
	Qiniu_Rio_ThreadModel threadModel;
} Qiniu_Rio_Settings;

QINIU_DLLAPI extern void Qiniu_Rio_SetSettings(Qiniu_Rio_Settings* v);

/*============================================================================*/
/* type Qiniu_Rio_PutExtra */

typedef struct _Qiniu_Rio_BlkputRet {
	const char* ctx;
	const char* checksum;
	Qiniu_Uint32 crc32;
	Qiniu_Uint32 offset;
	const char* host;
} Qiniu_Rio_BlkputRet;

#define QINIU_RIO_NOTIFY_OK 0
#define QINIU_RIO_NOTIFY_EXIT 1

typedef int (*Qiniu_Rio_FnNotify)(void* recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret);
typedef int (*Qiniu_Rio_FnNotifyErr)(void* recvr, int blkIdx, int blkSize, Qiniu_Error err);

typedef struct _Qiniu_Rio_PutExtra {
	const char* mimeType;
	int chunkSize;
	int tryTimes;
	void* notifyRecvr;
	Qiniu_Rio_FnNotify notify;
	Qiniu_Rio_FnNotifyErr notifyErr;
	Qiniu_Rio_BlkputRet* progresses;
	size_t blockCnt;
	Qiniu_Rio_ThreadModel threadModel;

	// For those file systems that save file name as Unicode strings,
	// use this field to name the local file name in UTF-8 format for CURL.
	const char* localFileName;

	// For those who want to invoke a upload callback on the business server
	// which returns a JSON object.
	void* callbackRet;
	Qiniu_Error (*callbackRetParser)(void*, Qiniu_Json*);

	// Use xVarsList to pass user defined variables and xVarsCount to pass the count of them.
	//
	// (extra->xVarsList[i])[0] set as the variable name, e.g. "x:Price".
	// **NOTICE**: User defined variable's name MUST starts with a prefix string "x:".
    //
	// (extra->xVarsList[i])[1] set as the value, e.g. "priceless".
	const char* (*xVarsList)[2];
	int xVarsCount;

	// For those who want to send request to specific host.
	const char* upHost;
} Qiniu_Rio_PutExtra;

/*============================================================================*/
/* type Qiniu_Rio_PutRet */

typedef Qiniu_Io_PutRet Qiniu_Rio_PutRet;

/*============================================================================*/
/* func Qiniu_Rio_BlockCount */

QINIU_DLLAPI extern int Qiniu_Rio_BlockCount(Qiniu_Int64 fsize);

/*============================================================================*/
/* func Qiniu_Rio_PutXXX */

#ifndef QINIU_UNDEFINED_KEY
#define QINIU_UNDEFINED_KEY		NULL
#endif

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rio_Put(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra* extra);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rio_PutFile(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, Qiniu_Rio_PutExtra* extra);

/*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // QINIU_RESUMABLE_IO_H
