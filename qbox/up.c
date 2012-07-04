/*
 ============================================================================
 Name        : up.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include <zlib.h>
#include "up.h"

/*============================================================================*/
/* func QBox_RS_MakeBlock, QBox_RS_PutBlock, QBox_RS_MergeBlock */

static const QBox_Off_t QBOX_RS_BLOCKSIZE = (4 * 1024 * 1024); /* Default block size is 4MB */
static const QBox_Off_t QBOX_RS_CHUNKSIZE = (32 * 1024); /* Default chunk size is 32KB */
static const int QBOX_RS_BLOCK_CHECKSUM_SIZE = 20;

QBox_Error QBox_RS_SplitFile(QBox_Client* self, QBox_RS_BlockSet* blockSet, QBox_Off_t fsize)
{
    QBox_Error err;
    int i = 0;

    if (blockSet == NULL) {
        err.code = 400;
        err.message = "Invalid argument";
        return err;
    }

    blockSet->fsize = fsize;

    blockSet->lastBlockSize = blockSet->fsize % QBOX_RS_BLOCKSIZE;
    blockSet->count = blockSet->fsize / QBOX_RS_BLOCKSIZE;

    if (blockSet->lastBlockSize > 0) {
        blockSet->count += 1;
    }

    blockSet->blocks = calloc(sizeof(*(blockSet->blocks)), blockSet->count);

    for (i = 0; i < blockSet->count; ++i) {
        blockSet->blocks[i].set = blockSet;
    } /* for */

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_InitBlockData(QBox_RS_BlockData* blockData, int blockIndex, QBox_Off_t chunkSize)
{
    QBox_Error err;
    int bsize_len = 0;

    blockData->blockIndex = blockIndex;
    blockData->ctx = NULL;
    blockData->cksum = NULL;

    blockData->blockOffset = blockIndex * QBOX_RS_BLOCKSIZE;
    blockData->chunkOffset = blockData->blockOffset;

    if (blockData->blockIndex == blockData->set->count - 1) {
        /* It's the last block */
        blockData->blockEnd = blockData->blockOffset + blockData->set->lastBlockSize;
    }
    else {
        blockData->blockEnd = blockData->blockOffset + QBOX_RS_BLOCKSIZE;
    }

    bsize_len = sizeof(blockData->bsize_str);

    bzero(blockData->bsize_str, bsize_len);
    QBox_snprintf(blockData->bsize_str, bsize_len, "%lu", blockData->blockEnd - blockData->blockOffset);

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_MakeBlock(QBox_Client* self, QBox_RS_BlockSet* blockSet,
    int blockIndex, QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog)
{
    QBox_Error err;
    QBox_Off_t rest = 0;
    QBox_Off_t stepSize = 0;
    QBox_Off_t chunkSize = 0;
    QBox_RS_BlockData* blockData = &blockSet->blocks[blockIndex];
    char* chunk = NULL;

    chunkSize = (io->chunkSize > 0) ? io->chunkSize : QBOX_RS_CHUNKSIZE;

    err = QBox_RS_InitBlockData(blockData, blockIndex, chunkSize);

    rest = blockData->blockEnd - blockData->chunkOffset;
    stepSize = (rest > chunkSize) ? chunkSize : rest;
    chunk = malloc(stepSize);

    /* Put the first chunk */
    err = QBox_RS_PutChunk(self, blockData, 0, chunk, stepSize, io, prog);
    if (err.code != 200) {
        free(chunk);
        return err;
    }

    blockData->chunkOffset += stepSize;
    free(chunk);

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_PutBlock(QBox_Client* self, QBox_RS_BlockSet* blockSet,
    int blockIndex, QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog)
{
    QBox_Error err;
    QBox_Off_t rest = 0;
    QBox_Off_t offset = 0;
    QBox_Off_t stepSize = 0;
    QBox_Off_t chunkSize = 0;
    QBox_RS_BlockData* blockData = &blockSet->blocks[blockIndex];
    char* chunk = NULL;

    chunkSize = (io->chunkSize > 0) ? io->chunkSize : QBOX_RS_CHUNKSIZE;

    rest = blockData->blockEnd - blockData->chunkOffset;

    if (rest > 0) {
        stepSize = (rest > chunkSize) ? chunkSize : rest;
        chunk = malloc(stepSize);

        do {
            err = QBox_RS_PutChunk(self,
                blockData,
                blockData->chunkOffset - blockData->blockOffset, /* offset from the beginning of the current block */
                chunk,
                stepSize,
                io,
                prog);
            if (err.code != 200) {
                free(chunk);
                return err;
            }

            rest -= stepSize;
            blockData->chunkOffset += stepSize;
            stepSize = (rest > chunkSize) ? chunkSize : rest;
        } while (rest > 0);

        free(chunk);
    } /* if */

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_PutChunk(QBox_Client* self, QBox_RS_BlockData* blockData,
    QBox_Off_t chunkOffset, char* chunk, QBox_Off_t size,
    QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog)
{
    QBox_Error err;
    char offsetStr[128];

	char* mimeEncoded = NULL;
	cJSON* root = NULL;
    char* url = NULL;
    unsigned long chunk_crc32 = 0L;
    int keepGoing;
    QBox_Int64 ret_crc32 = 0;

    if (io->readChunk(io->data, blockData->blockOffset + chunkOffset, chunk, &size) == 0) {
        err.code = 400;
        err.message = "Failed in reading file";
        return err;
    }

	mimeEncoded = QBox_String_Encode("application/octet-stream");

    if (blockData->ctx == NULL) {
        /* Make a new block and put the first chunk */
	    url = QBox_String_Concat(QBOX_UP_HOST, "/mkblk/", blockData->bsize_str, "/mime/", mimeEncoded, NULL);
    }
    else {
        /* Put other chunks */
        bzero(offsetStr, sizeof(offsetStr));
        QBox_snprintf(offsetStr, sizeof(offsetStr), "%lu", chunkOffset);

        url = QBox_String_Concat(QBOX_UP_HOST, "/bput/", blockData->ctx, "/", offsetStr, "/mime/", mimeEncoded, NULL);
    }

	err = QBox_Client_CallWithBuffer(self, &root, url, chunk, size);
	free(url);

	if (err.code == 200) {
		blockData->ctx = QBox_Json_GetString(root, "ctx", NULL);
		blockData->cksum = QBox_Json_GetString(root, "checksum", NULL);

		ret_crc32 = QBox_Json_GetInt64(root, "crc32", 0);
        chunk_crc32 = crc32(0L, NULL, 0);
        chunk_crc32 = crc32(chunk_crc32, chunk, size);

        if (chunk_crc32 != ret_crc32) {
            err.code = 400;
            err.message = "CRC32 check failed";
        }
        else {
            err.code = 200;
            err.message = "OK";
        }
	}

    if (prog && prog->callback) {
        keepGoing = prog->callback(prog->data,
                                   blockData->blockIndex,
                                   blockData->ctx,
                                   blockData->cksum,
                                   chunkOffset,
                                   err);

        if (!keepGoing && err.code == 200) {
            err.code = 499;
            err.message = "Aborted by client";
        }
    }

    return err;
}

QBox_Error QBox_RS_MakeFile(QBox_Client* self, QBox_RS_BlockSet* blockSet,
    const char* entry, const char* mimeType, const char* customMeta,
    const char* callbackParams)
{
    QBox_Error err;
    char fsize_str[256];
    int fsize_len = sizeof(fsize_str);
    int i = 0;
    char* cksum_buffer = NULL;
    int cksum_size = 20 * blockSet->count;
	cJSON* root = NULL;
    char* offset = NULL;
    char* url = NULL;
    char* cksum = NULL;
    char* params = NULL;
    char* params2 = NULL;
    char* mimeEncoded = NULL;
    char* metaEncoded = NULL;
    char* entryEncoded = NULL;

    cksum_buffer = calloc(20, blockSet->count);

    for (i = 0; i < blockSet->count; ++i) {
        /* Concatenate all ckecksum strings */
        offset = cksum_buffer + 20 * i;
        cksum = QBox_String_Decode(blockSet->blocks[i].cksum);
        memcpy(offset, cksum, 20);
        free(cksum);
    } /* for */

    bzero(fsize_str, fsize_len);
    QBox_snprintf(fsize_str, fsize_len, "%lu", blockSet->fsize);

    entryEncoded = QBox_String_Encode(entry);

	if (mimeType == NULL) {
		mimeType = "application/octet-stream";
	}
	mimeEncoded = QBox_String_Encode(mimeType);

    params = QBox_String_Concat("/mimeType/", mimeEncoded, NULL);

    if (customMeta != NULL) {
        metaEncoded = QBox_String_Encode(customMeta);
        params2 = QBox_String_Concat(params, "/meta/", metaEncoded, NULL);
        free(metaEncoded);
        free(params);
        params = params2;
    }
    if (callbackParams != NULL) {
        params2 = QBox_String_Concat(params, "/params/", callbackParams, NULL);
        free(params);
        params = params2;
    }

    url = QBox_String_Concat(QBOX_UP_HOST, "/rs-mkfile/", entryEncoded, "/fsize/", fsize_str, params, NULL);

	err = QBox_Client_CallWithBuffer(self, &root, url, cksum_buffer, cksum_size);
	free(url);

    err.code = 200;
    err.message = "OK";
    return err;
}

void QBox_RS_CleanupBlockSet(QBox_RS_BlockSet* blockSet)
{
    if (blockSet) {
        free(blockSet->blocks);
    }
}

QBox_Error QBox_RS_PutFile_ByUpToken(
    QBox_Client* self, const char* bucket, const char* key,
    const char* mimeType, const char* customMeta,
    const char* callbackParams, QBox_RS_BlockIO* io, QBox_RS_BlockProgress* prog)
{
    QBox_Error err;
    QBox_RS_BlockSet blockSet;
    char* entry = NULL;
    int i = 0;

    err = QBox_RS_SplitFile(self, &blockSet, io->fileSize);
    if (err.code != 200) {
        QBox_RS_CleanupBlockSet(&blockSet);
        return err;
    }

    for (i = 0; i < blockSet.count; ++i) {
        err = QBox_RS_MakeBlock(self, &blockSet, i, io, prog);
        if (err.code != 200) {
            QBox_RS_CleanupBlockSet(&blockSet);
            return err;
        }

        err = QBox_RS_PutBlock(self, &blockSet, i, io, prog);
        if (err.code != 200) {
            QBox_RS_CleanupBlockSet(&blockSet);
            return err;
        }
    } /* for */

    entry = QBox_String_Concat(bucket, ":", key, NULL);
    err = QBox_RS_MakeFile(self, &blockSet, entry, mimeType, customMeta, callbackParams);
    free(entry);

    QBox_RS_CleanupBlockSet(&blockSet);
    return err;
}
