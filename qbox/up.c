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

static const QBox_Int64 QBOX_UP_BLOCK_SIZE = (4 * 1024 * 1024); /* Default block size is 4MB */

typedef struct _QBox_UP_chunkReader {
    QBox_Int64 blkEndOffset;
    QBox_Int64 chkOffset;
    int chkSize;

    QBox_ReaderAt f;
} QBox_UP_chunkReader;

static QBox_Error QBox_UP_Chunkput(QBox_Client* self, QBox_UP_PutRet* ret,
    QBox_Reader body, int bodyLength,
    char* chkBuf, int chkSize, const char* url)
{
    QBox_Error err;
	char* mimeEncoded = NULL;
    char* url2 = NULL;
	cJSON* root = NULL;
    unsigned long chunk_crc32 = 0L;

    if (body.Read(chkBuf, chkSize, 1, body.self) == 0) {
        err.code = 400;
        err.message = "Failed in reading file";
        return err;
    }

	mimeEncoded = QBox_String_Encode("application/octet-stream");
	url2 = QBox_String_Concat(url, "/mime/", mimeEncoded, NULL);
    free(mimeEncoded);

	err = QBox_Client_CallWithBuffer(self, &root, url2, chkBuf, chkSize);
	free(url2);

	if (err.code == 200) {
		ret->ctx = QBox_Json_GetString(root, "ctx", NULL);
		ret->checksum = QBox_Json_GetString(root, "checksum", NULL);

		ret->crc32 = QBox_Json_GetInt64(root, "crc32", 0);
        chunk_crc32 = crc32(0L, NULL, 0);
        chunk_crc32 = crc32(chunk_crc32, chkBuf, chkSize);

        if (chunk_crc32 != ret->crc32) {
            err.code = 400;
            err.message = "Failed in verifying CRC32";
        }
        else {
            err.code = 200;
            err.message = "OK";
        }
	}

    return err;
}

/*============================================================================*/

QBox_Error QBox_UP_Mkblock(QBox_Client* self, QBox_UP_PutRet* ret,
    int blkSize, QBox_Reader body, int bodyLength)
{
    QBox_Error err;
    char blkSizeStr[128];
    int chkRealSize = 0;
    char* chkBuf = NULL;
    char* url = NULL;
    QBox_UP_chunkReader* chkReader = (QBox_UP_chunkReader*) body.self;
    
    bzero(blkSizeStr, sizeof(blkSizeStr));
    QBox_snprintf(blkSizeStr, sizeof(blkSizeStr), "%d", blkSize);
	url = QBox_String_Concat(QBOX_UP_HOST, "/mkblk/", blkSizeStr, NULL);

    chkRealSize = (blkSize > chkReader->chkSize) ? chkReader->chkSize : blkSize;
    chkBuf = malloc(chkRealSize);

    err = QBox_UP_Chunkput(self, ret, body, bodyLength, chkBuf, chkRealSize, url);
    free(chkBuf);
    free(url);

    return err;
}

/*============================================================================*/

QBox_Error QBox_UP_Blockput(QBox_Client* self, QBox_UP_PutRet* ret,
    const char* ctx, int offset, QBox_Reader body, int bodyLength)
{
    QBox_Error err;
    char offsetStr[128];
    int chkRealSize = 0;
    char* chkBuf = NULL;
    char* url = NULL;
    QBox_UP_chunkReader* chkReader = (QBox_UP_chunkReader*) body.self;
    
    bzero(offsetStr, sizeof(offsetStr));
    QBox_snprintf(offsetStr, sizeof(offsetStr), "%d", offset);

    url = QBox_String_Concat(QBOX_UP_HOST, "/bput/", ctx, "/", offsetStr, NULL);

    chkRealSize = chkReader->blkEndOffset - chkReader->chkOffset;
    if (chkRealSize > chkReader->chkSize) {
        chkRealSize = chkReader->chkSize;
    }

    chkBuf = malloc(chkRealSize);

    err = QBox_UP_Chunkput(self, ret, body, bodyLength, chkBuf, chkRealSize, url);
    free(chkBuf);
    free(url);

    return err;
}

/*============================================================================*/
static size_t QBox_UP_read(void *buf, size_t sz, size_t n, void *self)
{
    QBox_UP_chunkReader* chkReader = (QBox_UP_chunkReader*) self;

    if (chkReader->chkOffset < chkReader->blkEndOffset) {
        chkReader->f.ReadAt(chkReader->f.self, buf, sz, chkReader->chkOffset);
        chkReader->chkOffset += sz;
        return n;
    }

    return 0;
}

QBox_Error QBox_UP_ResumableBlockput(QBox_Client* self, QBox_UP_PutRet* ret,
	QBox_ReaderAt f, int blkIndex, int blkSize, int chkSize, int retryTimes,
	QBox_UP_BlockProgress* blkProg, QBox_UP_FnChunkNotify chunkNotify, void* notifyParams)
{
    QBox_Error err;
    QBox_Reader ri;
    QBox_UP_chunkReader chkReader;
    int i = 0;

    /* Do some initialization */
    chkReader.blkEndOffset = (blkIndex * QBOX_UP_BLOCK_SIZE) + blkSize;
    chkReader.chkOffset    = (blkIndex * QBOX_UP_BLOCK_SIZE) + ret->uploadedSize;
    chkReader.chkSize      = chkSize;
    chkReader.f            = f;

    ri.Read = QBox_UP_read;
    ri.self = &chkReader;

    /* Try make block and put the first chunk */
    if (ret->uploadedSize == 0) {
        for (i = 0; i < retryTimes; ++i) {
            err = QBox_UP_Mkblock(self, ret, blkSize, ri, blkSize);

            if (err.code == 200) {
                if (chunkNotify) {
                    blkProg->ctx      = (char *)ret->ctx;
                    blkProg->offset   = ret->uploadedSize;
                    blkProg->restSize = blkSize - ret->uploadedSize;
                    blkProg->errCode  = err.code;

                    chunkNotify(notifyParams, blkIndex, blkProg);
                }

                break;
            }
        } /* for retry */

        if (err.code != 200) {
            return err;
        }

        ret->uploadedSize = blkSize - (chkReader.blkEndOffset - chkReader.chkOffset);
    } /* make block */

    /* Try put block */
    while (ret->uploadedSize < blkSize) {
        for (i = 0; i < retryTimes; ++i) {
            err = QBox_UP_Blockput(self, ret, ret->ctx, ret->uploadedSize, ri, blkSize);

            if (err.code == 200) {
                if (chunkNotify) {
                    blkProg->ctx      = (char *)ret->ctx;
                    blkProg->offset   = ret->uploadedSize;
                    blkProg->restSize = blkSize - ret->uploadedSize;
                    blkProg->errCode  = err.code;

                    chunkNotify(notifyParams, blkIndex, blkProg);
                }

                break;
            }
        } /* for retry */

        if (err.code != 200) {
            return err;
        }

        ret->uploadedSize = blkSize - (chkReader.blkEndOffset - chkReader.chkOffset);
    } /* while putting block */

    err.code = 200;
    err.message = "OK";
    return err;
}

/*============================================================================*/

QBox_Error QBox_UP_Mkfile(
	QBox_Client* self, QBox_Json** ret,
    const char* cmd, const char* entry, const char* mimeType,
	QBox_Int64 fsize, const char* params, const char* callbackParams,
	QBox_UP_Checksum* checksums, int blkCount)
{
    QBox_Error err;
    char fsizeStr[256];
    int i = 0;
    char* cksumBuf = NULL;
    int cksumSize = 20 * blkCount;
    char* url = NULL;
    char* cksum = NULL;
    char* offset = NULL;
    char* params2 = NULL;
    char* paramsFinal = NULL;
    char* strEncoded = NULL;

    cksumBuf = calloc(20, blkCount);

    for (i = 0; i < blkCount; ++i) {
        /* Concatenate all ckecksum strings */
        offset = cksumBuf + 20 * i;
        cksum = QBox_String_Decode(checksums[i].value);
        memcpy(offset, cksum, 20);
        free(cksum);
    } /* for */

	if (mimeType == NULL) {
		mimeType = "application/octet-stream";
	}
	strEncoded = QBox_String_Encode(mimeType);

    if (params == NULL) {
        params = "";
    }

    paramsFinal = QBox_String_Concat(params, "/mimeType/", strEncoded, NULL);
    free(strEncoded);

    if (callbackParams != NULL) {
        params2 = QBox_String_Concat(paramsFinal, "/params/", callbackParams, NULL);
        free(paramsFinal);
        paramsFinal = params2;
    }

    strEncoded = QBox_String_Encode(entry);
    bzero(fsizeStr, sizeof(fsizeStr));
    QBox_snprintf(fsizeStr, sizeof(fsizeStr), "%llu", fsize);

    url = QBox_String_Concat(QBOX_UP_HOST, cmd, strEncoded, "/fsize/", fsizeStr, paramsFinal, NULL);
	free(paramsFinal);
	free(strEncoded);

	err = QBox_Client_CallWithBuffer(self, ret, url, cksumBuf, cksumSize);
	free(url);

    return err;
}

/*============================================================================*/

QBox_UP_Progress* QBox_UP_NewProgress(QBox_Int64 fsize)
{
    QBox_UP_Progress* prog = NULL;

    prog = malloc(sizeof(*prog));

    if (prog != NULL) {
        prog->blockCount = fsize / QBOX_UP_BLOCK_SIZE;

        if (fsize % QBOX_UP_BLOCK_SIZE > 0) {
            prog->blockCount += 1;
        }

        prog->checksums = malloc(sizeof(*(prog->checksums)) * prog->blockCount);
        prog->progs     = malloc(sizeof(*(prog->progs))     * prog->blockCount);
    }

    return prog;
}

void QBox_UP_Progress_Release(QBox_UP_Progress* prog)
{
    if (prog != NULL) {
        free(prog->progs);
        free(prog->checksums);
        free(prog);
    }
}

/*============================================================================*/

QBox_Error QBox_UP_Put(QBox_Client* self, QBox_ReaderAt f,
    QBox_Int64 fsize, QBox_UP_Progress* prog,
	QBox_UP_FnBlockNotify blockNotify, QBox_UP_FnChunkNotify chunkNotify, void* notifyParams)
{
    QBox_Error err;
    QBox_UP_PutRet ret;
    QBox_Json* root; 
    int blkIndex = 0;
    int blkSize = 0;
    QBox_Int64 rest = 0;

    rest = fsize;
    for (blkIndex = 0; blkIndex < prog->blockCount; ++blkIndex) {
        ret.ctx          = NULL;
        ret.checksum     = NULL;
        ret.crc32        = 0;
        ret.uploadedSize = 0;

        blkSize = (rest > QBOX_UP_BLOCK_SIZE) ? QBOX_UP_BLOCK_SIZE : rest;

        err = QBox_UP_ResumableBlockput(
            self,
            &ret,
            f,
            blkIndex,
            blkSize,
            QBOX_PUT_CHUNK_SIZE,
            QBOX_PUT_RETRY_TIMES,
            &prog->progs[blkIndex],
            chunkNotify,
            notifyParams
        );

        if (err.code != 200) {
            return err;
        }

        memcpy(prog->checksums[blkIndex].value, ret.checksum, strlen(ret.checksum));

        if (blockNotify) {
            blockNotify(notifyParams, blkIndex, &prog->checksums[blkIndex]);
        }

        rest -= blkSize;
    } /* for */

    err.code = 200;
    err.message = "OK";
    return err;
}
