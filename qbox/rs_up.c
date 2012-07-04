/*
 ============================================================================
 Name        : rs_up.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */
 
#include "rs.h"

/*============================================================================*/

QBox_Error QBox_RS_ResumablePut(
	QBox_Client* self, QBox_RS_PutRet* ret, QBox_UP_Progress* prog,
	QBox_UP_FnBlockNotify blockNotify, QBox_UP_FnChunkNotify chunkNotify, void* notifyParams,
	const char* entryURI, const char* mimeType, QBox_ReaderAt f, QBox_Int64 fsize,
	const char* customMeta, const char* callbackParams)
{
}

/*============================================================================*/

