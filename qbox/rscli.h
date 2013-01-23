/*
 ============================================================================
 Name        : rscli.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_RSCLI_H
#define QBOX_RSCLI_H

#include "base.h"

/*============================================================================*/
/* func QBox_RSCli_PutFile */

QBox_Error QBox_RSCli_PutFile(
	QBox_Buffer* resp, const char* url, const char* tableName, const char* key,
	const char* mimeType, const char* localFile, const char* customMeta, const char* callbackParams);

QBox_Error QBox_RSCli_PutStream( 
    QBox_Buffer* resp, const char* url, const char* tableName, const char* key, 
    const char* mimeType, const char* pStream, int bytes, const char* customMeta, const char* callbackParams);

/*============================================================================*/

#endif // QBOX_RSCLI_H

