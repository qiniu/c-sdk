/*
 ============================================================================
 Name        : io.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_IO_H
#define QINIU_IO_H

#include "http.h"

/*============================================================================*/
/* type Qiniu_Io_PutExtra */

typedef struct _Qiniu_Io_PutExtra {
	const char* callbackParams;	// 当 uptoken 指定了 CallbackUrl，则 CallbackParams 必须非空
	const char* bucket; 		// 当前是必选项，但未来会去掉
	const char* customMeta;		// 可选。用户自定义 Meta，不能超过 256 字节
	const char* mimeType;		// 可选。在 uptoken 没有指定 DetectMime 时，用户客户端可自己指定 MimeType
} Qiniu_Io_PutExtra;

/*============================================================================*/
/* type Qiniu_Io_PutRet */

typedef struct _Qiniu_Io_PutRet {
	const char* hash;			// 如果 uptoken 没有指定 ReturnBody，那么返回值是标准的 PutRet 结构
} Qiniu_Io_PutRet;

/*============================================================================*/
/* func Qiniu_Io_PutXXX */

Qiniu_Error Qiniu_Io_PutFile(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, Qiniu_Io_PutExtra* extra);

Qiniu_Error Qiniu_Io_PutBuffer(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* buf, size_t fsize, Qiniu_Io_PutExtra* extra);

/*============================================================================*/

#endif // QINIU_IO_H

