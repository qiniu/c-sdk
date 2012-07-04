/*
 ============================================================================
 Name        : up.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_UP_H
#define QBOX_UP_H

#include "oauth2.h"
#include <stdlib.h>

/*============================================================================*/

typedef struct _QBox_UP_BlockProgress {
	char* ctx;
	int offset;
	int restSize;
	int errCode;
} QBox_UP_BlockProgress;

typedef void (*QBox_UP_FnChunkNotify)(void* self, int blockIdx, QBox_UP_BlockProgress* prog);

typedef struct _QBox_UP_PutRet {
	const char* ctx;
	const char* checksum;
	QBox_Uint32 crc32;
} QBox_UP_PutRet;

QBox_Error QBox_UP_Mkblock(
	QBox_Client* self, QBox_UP_PutRet* ret, int blockSize, QBox_Reader body, int bodyLength);

QBox_Error QBox_UP_Blockput(
	QBox_Client* self, QBox_UP_PutRet* ret, const char* ctx, int offset, QBox_Reader body, int bodyLength);

QBox_Error QBox_UP_ResumableBlockput(
	QBox_Client* self, QBox_UP_PutRet* ret,
	QBox_ReaderAt f, int blockIdx, int blkSize, int chunkSize, int retryTimes,
	QBox_UP_BlockProgress* prog,
	QBox_UP_FnChunkNotify chunkNotify, void* notifyParams);

/*============================================================================*/

typedef struct _QBox_UP_Checksum {
	char value[28];
} QBox_UP_Checksum;

QBox_Error QBox_UP_Mkfile(
	QBox_Client* self, QBox_Json** ret, const char* cmd, const char* entry,
	QBox_Int64 fsize, const char* params, const char* callbackParams,
	QBox_UP_Checksum* checksums, int blockCount);

/*============================================================================*/

typedef struct _QBox_UP_Progress {
	QBox_UP_Checksum* checksums;
	QBox_UP_BlockProgress* progs;
	int blockCount;
} QBox_UP_Progress;

QBox_UP_Progress* QBox_UP_NewProgress(QBox_Int64 fsize);
void QBox_UP_Progress_Release(QBox_UP_Progress* prog);

typedef void (*QBox_UP_FnBlockNotify)(void* self, int blockIdx, QBox_UP_Checksum* checksum);

QBox_Error QBox_UP_Put(
	QBox_Client* self, QBox_ReaderAt f, QBox_Int64 fsize, QBox_UP_Progress* prog,
	QBox_UP_FnBlockNotify blockNotify, QBox_UP_FnChunkNotify chunkNotify, void* notifyParams);

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
    const char* entry,  const char* mimeType, const char* customMeta,
    const char* callbackParams);
void QBox_RS_CleanupBlockSet(QBox_RS_BlockSet* blockSet);

QBox_Error QBox_RS_PutFile_ByUpToken(
    QBox_Client* self, const char* bucket, const char* key,
    const char* mimeType, const char* customMeta,
    const char* callbackParams, QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog);

/*============================================================================*/

#endif /* QBOX_UP_H */

