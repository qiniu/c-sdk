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

/*============================================================================*/

#define Qiniu_Rio_InvalidCtx			701
#define Qiniu_Rio_UnmatchedChecksum		9900
#define Qiniu_Rio_InvalidPutProgress	9901
#define Qiniu_Rio_PutFailed				9902

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

/*============================================================================*/
/* type Qiniu_Rio_ThreadModel */

typedef struct _Qiniu_Rio_ThreadModel_Itbl {
	Qiniu_Rio_WaitGroup (*WaitGroup)(void* self);
	Qiniu_Client* (*ClientTls)(void* self, Qiniu_Client* mc);
	void (*RunTask)(void* self, void (*task)(void* params), void* params);
} Qiniu_Rio_ThreadModel_Itbl;

typedef struct _Qiniu_Rio_ThreadModel {
	void* self;
	Qiniu_Rio_ThreadModel_Itbl* itbl;
} Qiniu_Rio_ThreadModel;

extern Qiniu_Rio_ThreadModel Qiniu_Rio_ST;

/*============================================================================*/
/* type Qiniu_Rio_Settings */

typedef struct _Qiniu_Rio_Settings {
	int taskQsize;		// 可选。任务队列大小。为 0 表示取 Workers * 4。
	int workers;		// 并行 Goroutine 数目。
	int chunkSize;		// 默认的Chunk大小，不设定则为256k
	int tryTimes;		// 默认的尝试次数，不设定则为3
	Qiniu_Rio_ThreadModel threadModel;
} Qiniu_Rio_Settings;

void Qiniu_Rio_SetSettings(Qiniu_Rio_Settings* v);

/*============================================================================*/
/* type Qiniu_Rio_PutExtra */

typedef struct _Qiniu_Rio_BlkputRet {
	const char* ctx;
	const char* checksum;
	Qiniu_Uint32 crc32;
	Qiniu_Uint32 offset;
	const char* host;
} Qiniu_Rio_BlkputRet;

typedef void (*Qiniu_Rio_FnNotify)(void* recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret);
typedef void (*Qiniu_Rio_FnNotifyErr)(void* recvr, int blkIdx, int blkSize, Qiniu_Error err);

typedef struct _Qiniu_Rio_PutExtra {
	const char* callbackParams;	// 当 uptoken 指定了 CallbackUrl，则 CallbackParams 必须非空
	const char* bucket; 		// 当前是必选项，但未来会去掉
	const char* customMeta;		// 可选。用户自定义 Meta，不能超过 256 字节
	const char* mimeType;		// 可选。在 uptoken 没有指定 DetectMime 时，用户客户端可自己指定 MimeType
	int chunkSize;				// 可选。每次上传的Chunk大小
	int tryTimes;				// 可选。尝试次数
	void* notifyRecvr;
	Qiniu_Rio_FnNotify notify;		 // 可选。进度提示（注意多个block是并行传输的）
	Qiniu_Rio_FnNotifyErr notifyErr;
	Qiniu_Rio_BlkputRet* progresses; // 可选。上传进度
	size_t blockCnt;
	Qiniu_Rio_ThreadModel threadModel;
} Qiniu_Rio_PutExtra;

/*============================================================================*/
/* type Qiniu_Rio_PutRet */

typedef Qiniu_Io_PutRet Qiniu_Rio_PutRet;

/*============================================================================*/
/* func Qiniu_Rio_BlockCount */

int Qiniu_Rio_BlockCount(Qiniu_Int64 fsize);

/*============================================================================*/
/* func Qiniu_Rio_PutXXX */

#ifndef QINIU_UNDEFINED_KEY
#define QINIU_UNDEFINED_KEY		NULL
#endif

Qiniu_Error Qiniu_Rio_Put(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra* extra);

Qiniu_Error Qiniu_Rio_PutFile(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, Qiniu_Rio_PutExtra* extra);

/*============================================================================*/

#pragma pack()

#endif // QINIU_RESUMABLE_IO_H

