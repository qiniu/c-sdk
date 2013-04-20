/*
 ============================================================================
 Name        : io.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_IO_H
#define QBOX_IO_H

#include "base.h"
#include "oauth2.h"

/*============================================================================*/
/* type QBox_Io_PutExtra */

typedef struct _QBox_Io_PutExtra {
	const char* callbackParams;	// 当 uptoken 指定了 CallbackUrl，则 CallbackParams 必须非空
	const char* bucket; 		// 当前是必选项，但未来会去掉
	const char* customMeta;		// 可选。用户自定义 Meta，不能超过 256 字节
	const char* mimeType;		// 可选。在 uptoken 没有指定 DetectMime 时，用户客户端可自己指定 MimeType
} QBox_Io_PutExtra;

/*============================================================================*/
/* type QBox_Io_PutRet */

typedef struct _QBox_Io_PutRet {
	const char* hash;			// 如果 uptoken 没有指定 ReturnBody，那么返回值是标准的 PutRet 结构
} QBox_Io_PutRet;

/*============================================================================*/
/* func QBox_Io_PutXXX */

QBox_Error QBox_Io_Put(
	QBox_Client* self, QBox_Io_PutRet* ret,
	const char* uptoken, const char* key, QBox_Reader body, QBox_Int64 fsize, QBox_Io_PutExtra* extra);

QBox_Error QBox_Io_PutFile(
	QBox_Client* self, QBox_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, QBox_Io_PutExtra* extra);

QBox_Error QBox_Io_PutBuffer(
	QBox_Client* self, QBox_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* buf, size_t fsize, QBox_Io_PutExtra* extra);

/*============================================================================*/

#endif // QBOX_IO_H

