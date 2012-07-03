/*
 ============================================================================
 Name        : rs.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_RS_H
#define QBOX_RS_H

#include "oauth2.h"

/*============================================================================*/
/* func QBox_RS_PutAuth, QBox_RS_PutAuthEx */

typedef struct _QBox_RS_PutAuthRet {
	const char *url;
	QBox_Int64 expiresIn;
} QBox_RS_PutAuthRet;

QBox_Error QBox_RS_PutAuth(
	QBox_Client* self, QBox_RS_PutAuthRet* ret);

QBox_Error QBox_RS_PutAuthEx(
	QBox_Client* self, QBox_RS_PutAuthRet* ret, const char* callbackUrl, int expiresIn);

/*============================================================================*/
/* func QBox_RS_Get, QBox_RS_GetIfNotModified */

typedef struct _QBox_RS_GetRet {
	const char *url;
	const char* hash;
	const char* mimeType;
	QBox_Int64 fsize;
	QBox_Int64 expiresIn;
} QBox_RS_GetRet;

QBox_Error QBox_RS_Get(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName);

QBox_Error QBox_RS_GetIfNotModified(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName, const char* base);

/*============================================================================*/
/* func QBox_RS_Put, QBox_RS_PutFile */

typedef struct _QBox_RS_PutRet {
	const char* hash;
} QBox_RS_PutRet;

QBox_Error QBox_RS_Put(
	QBox_Client* self, QBox_RS_PutRet* ret, const char* tableName, const char* key,
	const char* mimeType, FILE* source, QBox_Int64 fsize, const char* customMeta);

QBox_Error QBox_RS_PutFile(
	QBox_Client* self, QBox_RS_PutRet* ret, const char* tableName, const char* key,
	const char* mimeType, const char* srcFile, const char* customMeta);

/*============================================================================*/
/* func QBox_RS_Stat */

typedef struct _QBox_RS_StatRet {
	const char* hash;
	const char* mimeType;
	QBox_Int64 fsize;	
	QBox_Int64 putTime;
} QBox_RS_StatRet;

QBox_Error QBox_RS_Stat(
	QBox_Client* self, QBox_RS_StatRet* ret, const char* tableName, const char* key);

/*============================================================================*/
/* func QBox_RS_Publish, QBox_RS_Unpublish */

QBox_Error QBox_RS_Publish(QBox_Client* self, const char* tableName, const char* domain);
QBox_Error QBox_RS_Unpublish(QBox_Client* self, const char* domain);

/*============================================================================*/
/* func QBox_RS_Delete */

QBox_Error QBox_RS_Delete(QBox_Client* self, const char* tableName, const char* key);

/*============================================================================*/
/* func QBox_RS_Drop */

QBox_Error QBox_RS_Drop(QBox_Client* self, const char* tableName);

/*============================================================================*/
/* func QBox_RS_MakeBlock, QBox_RS_PutBlock, QBox_RS_MergeBlock */
/* func QBox_RS_PutChunk */

typedef unsigned long QBox_Off_t;

typedef int (*QBox_RS_BlockCallback)(void *data, int blockIndex,
    const char* ctx, const char* cksum,
    QBox_Off_t chunkOffset, QBox_Error err);

typedef struct _QBox_RS_BlockIO {
    int (*readChunk)(void* data, size_t offset, void* chunk, size_t* size);
    void* data;

    QBox_Off_t fileSize;
    QBox_Off_t chunkSize;
} QBox_RS_BlockIO;

typedef struct _QBox_RS_BlockProgress {
    QBox_RS_BlockCallback callback;
    void* data;
} QBox_RS_BlockProgress;

struct _QBox_RS_BlockSet;

typedef struct _QBox_RS_BlockData {
    int blockIndex;

    const char* ctx;
    const char* cksum;

    QBox_Off_t blockOffset; /* from the beginning of the file */
    QBox_Off_t chunkOffset; /* from the beginning of the file */
    QBox_Off_t blockEnd;    /* from the beginning of the file */

    char bsize_str[16];
    struct _QBox_RS_BlockSet* set;
} QBox_RS_BlockData;

typedef struct _QBox_RS_BlockSet {
    QBox_Off_t fsize;

    int count;
    QBox_Off_t lastBlockSize;

    QBox_RS_BlockData* blocks;
} QBox_RS_BlockSet;

QBox_Error QBox_RS_InitBlockData(QBox_RS_BlockData* blockData, int blockIndex, QBox_Off_t chunkSize);
QBox_Error QBox_RS_PutChunk(QBox_Client* self, QBox_RS_BlockData* blockData,
    QBox_Off_t chunkOffset, char* chunk, QBox_Off_t size,
    QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog);

QBox_Error QBox_RS_MakeBlock(QBox_Client* self, QBox_RS_BlockSet* blockSet,
    int blockIndex, QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog);
QBox_Error QBox_RS_PutBlock(QBox_Client* self, QBox_RS_BlockSet* blockSet,
    int blockIndex, QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog);

QBox_Error QBox_RS_SplitFile(QBox_Client* self, QBox_RS_BlockSet* blockSet, QBox_Off_t fsize);
QBox_Error QBox_RS_MakeFile(QBox_Client* self, QBox_RS_BlockSet* blockSet,
    const char* entry, const char* mimeType, const char* customMeta,
    const char* callbackParams);
void QBox_RS_CleanupBlockSet(QBox_RS_BlockSet* blockSet);

QBox_Error QBox_RS_PutFile_ByUpToken(
    QBox_Client* self, const char* bucket, const char* key,
    const char* mimeType, const char* customMeta,
    const char* callbackParams, QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog);

/*============================================================================*/

#endif /* QBOX_RS_H */

