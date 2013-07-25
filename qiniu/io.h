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

#pragma pack(1)

/*============================================================================*/
/* type Qiniu_Io_PutExtra */

typedef struct _Qiniu_Io_PutExtraParam {
	const char* key; // 必须以 "x:" 开始
	const char* value;
	struct _Qiniu_Io_PutExtraParam* next;
} Qiniu_Io_PutExtraParam;

typedef struct _Qiniu_Io_PutExtra {
	Qiniu_Io_PutExtraParam* params;
	const char* mimeType;		// 可选。在 uptoken 没有指定 DetectMime 时，用户客户端可自己指定 MimeType
	Qiniu_Uint32 crc32;
	Qiniu_Uint32 checkCrc32;
        // CheckCrc == 0: 表示不进行 crc32 校验
        // CheckCrc == 1: 对于 Put 等同于 CheckCrc = 2；对于 PutFile 会自动计算 crc32 值
        // CheckCrc == 2: 表示进行 crc32 校验，且 crc32 值就是上面的 Crc32 变量
} Qiniu_Io_PutExtra;

/*============================================================================*/
/* type Qiniu_Io_PutRet */

typedef struct _Qiniu_Io_PutRet {
	const char* hash;			// 如果 uptoken 没有指定 ReturnBody，那么返回值是标准的 PutRet 结构
	const char* key;			// 如果传入的key未指定（UNDEFINED_KEY），那么返回服务端自动生成的key
} Qiniu_Io_PutRet;

/*============================================================================*/
/* func Qiniu_Io_PutXXX */

#ifndef QINIU_UNDEFINED_KEY
#define QINIU_UNDEFINED_KEY		NULL
#endif

Qiniu_Error Qiniu_Io_PutFile(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, Qiniu_Io_PutExtra* extra);

Qiniu_Error Qiniu_Io_PutBuffer(
	Qiniu_Client* self, Qiniu_Io_PutRet* ret,
	const char* uptoken, const char* key, const char* buf, size_t fsize, Qiniu_Io_PutExtra* extra);

/*============================================================================*/

#pragma pack()

#endif // QINIU_IO_H

