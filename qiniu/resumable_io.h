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

#include "oauth2.h"

/*============================================================================*/

#define Qiniu_Rio_InvalidCtx			701
#define Qiniu_Rio_UnmatchedChecksum		9900

/*============================================================================*/
/* type Qiniu_Rio_PutExtra */

typedef struct _Qiniu_Rio_BlkputRet {
	const char* ctx;
	const char* checksum;
	Qiniu_Uint32 crc32;
	Qiniu_Uint32 offset;
	const char* host;
} Qiniu_Rio_BlkputRet;

typedef (*Qiniu_Rio_FnNotify)(void* recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret);
typedef (*Qiniu_Rio_FnNotifyErr)(void* recvr, int blkIdx, int blkSize, Qiniu_Error err);

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
	size_t blkCount;
} Qiniu_Rio_PutExtra;

/*============================================================================*/
/* type Qiniu_Rio_PutRet */

typedef struct _Qiniu_Rio_PutRet {
	const char* hash;			// 如果 uptoken 没有指定 ReturnBody，那么返回值是标准的 PutRet 结构
} Qiniu_Rio_PutRet;

/*============================================================================*/
/* func Qiniu_Rio_PutXXX */

Qiniu_Error Qiniu_Rio_Put(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra* extra);

Qiniu_Error Qiniu_Rio_PutFile(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, Qiniu_Rio_PutExtra* extra);

/*============================================================================*/

#endif // QINIU_RESUMABLE_IO_H

