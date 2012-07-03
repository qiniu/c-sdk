/*
 ============================================================================
 Name        : rs.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include <zlib.h>
#include "rs.h"

/*============================================================================*/
/* func QBox_RS_PutAuth, QBox_RS_PutAuthEx */

QBox_Error QBox_RS_PutAuth(
	QBox_Client* self, QBox_RS_PutAuthRet* ret)
{
	QBox_Error err;
	cJSON* root;
	char* url = QBox_String_Concat2(QBOX_IO_HOST, "/put-auth/");

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", NULL);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

QBox_Error QBox_RS_PutAuthEx(
	QBox_Client* self, QBox_RS_PutAuthRet* ret, const char* callbackUrl, int expiresIn)
{
	QBox_Error err;
	cJSON* root;
	char* url;
	char* url2;
	char* callbackEncoded;

	char expires[32];
	QBox_snprintf(expires, 32, "%d", expiresIn);

	url = QBox_String_Concat3(QBOX_IO_HOST, "/put-auth/", expires);

	if (callbackUrl != NULL && *callbackUrl != '\0') {
		callbackEncoded = QBox_String_Encode(callbackUrl);
		url2 = QBox_String_Concat3(url, "/callback/", callbackEncoded);
		free(url);
		free(callbackEncoded);
		url = url2;
	}

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", NULL);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

/*============================================================================*/
/* func QBox_RS_Put, QBox_RS_PutFile */

QBox_Error QBox_RS_Put(
	QBox_Client* self, QBox_RS_PutRet* ret, const char* tableName, const char* key,
	const char* mimeType, FILE* source, QBox_Int64 fsize, const char* customMeta)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* customMetaEncoded;
	char* mimeEncoded;
	char* url;
	char* url2;

	if (mimeType == NULL) {
		mimeType = "application/octet-stream";
	}

	mimeEncoded = QBox_String_Encode(mimeType);
	url = QBox_String_Concat(QBOX_IO_HOST, "/rs-put/", entryURIEncoded, "/mime/", mimeEncoded, NULL);
	free(mimeEncoded);
	free(entryURIEncoded);

	if (customMeta != NULL && *customMeta != '\0') {
		customMetaEncoded = QBox_String_Encode(customMeta);
		url2 = QBox_String_Concat3(url, "/meta/", customMetaEncoded);
		free(url);
		free(customMetaEncoded);
		url = url2;
	}

	err = QBox_Client_CallWithBinary(self, &root, url, source, fsize);
	free(url);

	if (err.code == 200) {
		ret->hash = QBox_Json_GetString(root, "hash", NULL);
	}
	return err;
}

QBox_Error QBox_RS_PutFile(
	QBox_Client* self, QBox_RS_PutRet* ret, const char* tableName, const char* key,
	const char* mimeType, const char* srcFile, const char* customMeta)
{
	QBox_Error err;
	QBox_Int64 fsize;
	FILE* fp = fopen(srcFile, "rb");
	if (fp == NULL) {
		err.code = -1;
		err.message = "open source file failed";
		return err;
	}
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	err = QBox_RS_Put(self, ret, tableName, key, mimeType, fp, fsize, customMeta);
	fclose(fp);
	return err;
}

/*============================================================================*/
/* func QBox_RS_Get, QBox_RS_GetIfNotModified */

QBox_Error QBox_RS_Get(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/get/", entryURIEncoded);
	char* urlOld;
	char* attNameEncoded;

	free(entryURI);
	free(entryURIEncoded);

	if (attName != NULL) {
		attNameEncoded = QBox_String_Encode(attName);
		urlOld = url;
		url = QBox_String_Concat3(url, "/attName/", attNameEncoded);
		free(attNameEncoded);
		free(urlOld);
	}

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", "unknown");
		ret->hash = QBox_Json_GetString(root, "hash", NULL);
		ret->mimeType = QBox_Json_GetString(root, "mimeType", NULL);
		ret->fsize = QBox_Json_GetInt64(root, "fsize", 0);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

QBox_Error QBox_RS_GetIfNotModified(
	QBox_Client* self, QBox_RS_GetRet* ret, const char* tableName, const char* key, const char* attName, const char* base)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat(QBOX_RS_HOST, "/get/", entryURIEncoded, "/base/", base, NULL);
	char* urlOld;
	char* attNameEncoded;

	free(entryURI);
	free(entryURIEncoded);

	if (attName != 0) {
		attNameEncoded = QBox_String_Encode(attName);
		urlOld = url;
		url = QBox_String_Concat3(url, "/attName/", attNameEncoded);
		free(attNameEncoded);
		free(urlOld);
	}

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->url = QBox_Json_GetString(root, "url", "unknown");
		ret->hash = QBox_Json_GetString(root, "hash", 0);
		ret->mimeType = QBox_Json_GetString(root, "mimeType", 0);
		ret->fsize = QBox_Json_GetInt64(root, "fsize", 0);
		ret->expiresIn = QBox_Json_GetInt64(root, "expiresIn", 0);
	}
	return err;
}

/*============================================================================*/
/* func QBox_RS_Stat */

QBox_Error QBox_RS_Stat(
	QBox_Client* self, QBox_RS_StatRet* ret, const char* tableName, const char* key)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/stat/", entryURIEncoded);

	free(entryURI);
	free(entryURIEncoded);

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->hash = QBox_Json_GetString(root, "hash", 0);
		ret->mimeType = QBox_Json_GetString(root, "mimeType", 0);
		ret->fsize = QBox_Json_GetInt64(root, "fsize", 0);
		ret->putTime = QBox_Json_GetInt64(root, "putTime", 0);
	}
	return err;
}

/*============================================================================*/
/* func QBox_RS_Publish, QBox_RS_Unpublish */

QBox_Error QBox_RS_Publish(QBox_Client* self, const char* tableName, const char* domain)
{
	QBox_Error err;

	char* domainEncoded = QBox_String_Encode(domain);
	char* url = QBox_String_Concat(QBOX_RS_HOST, "/publish/", domainEncoded, "/from/", tableName, NULL);
	free(domainEncoded);

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

QBox_Error QBox_RS_Unpublish(QBox_Client* self, const char* domain)
{
	QBox_Error err;

	char* domainEncoded = QBox_String_Encode(domain);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/unpublish/", domainEncoded);
	free(domainEncoded);

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

/*============================================================================*/
/* func QBox_RS_Delete */

QBox_Error QBox_RS_Delete(QBox_Client* self, const char* tableName, const char* key)
{
	QBox_Error err;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/delete/", entryURIEncoded);

	free(entryURI);
	free(entryURIEncoded);

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

/*============================================================================*/
/* func QBox_RS_Drop */

QBox_Error QBox_RS_Drop(QBox_Client* self, const char* tableName)
{
	QBox_Error err;

	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/drop/", tableName);

	err = QBox_Client_CallNoRet(self, url);
	free(url);

	return err;
}

/*============================================================================*/
/* func QBox_RS_MakeBlock, QBox_RS_PutBlock, QBox_RS_MergeBlock */

static const QBox_Off_t QBOX_RS_BLOCKSIZE = (4 * 1024 * 1024); /* Default block size is 4MB */
static const QBox_Off_t QBOX_RS_CHUNKSIZE = (32 * 1024); /* Default chunk size is 32KB */
static const int QBOX_RS_BLOCK_CHECKSUM_SIZE = 20;

QBox_Error QBox_RS_SplitFile(QBox_Client* self, QBox_RS_BlockSet* blockSet, FILE *fp)
{
    QBox_Error err;
    QBox_Off_t cur_pos = 0;

    if (fp == NULL) {
        err.code = 400;
        err.message = "Invalid file handle";
        return err;
    }

    if (blockSet == NULL) {
        err.code = 400;
        err.message = "Invalid argument";
        return err;
    }

    cur_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    blockSet->fsize = ftell(fp);
    fseek(fp, cur_pos, SEEK_SET);

    blockSet->lastBlockSize = blockSet->fsize % QBOX_RS_BLOCKSIZE;
    blockSet->count = blockSet->fsize / QBOX_RS_BLOCKSIZE;

    if (blockSet->lastBlockSize > 0) {
        blockSet->count += 1;
    }

    blockSet->blocks = calloc(sizeof(*(blockSet->blocks)), blockSet->count);

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_InitBlockData(QBox_RS_BlockSet* blockSet, QBox_RS_BlockData* blockData,
    int blockIndex, QBox_Off_t chunkSize)
{
    QBox_Error err;
    int bsize_len = 0;
    int ret = 0;

    blockData->index = blockIndex;
    blockData->begin = blockIndex * QBOX_RS_BLOCKSIZE;

    if (blockData->index == blockSet->count - 1) {
        /* It's the last block */
        blockData->end = blockData->begin + blockSet->lastBlockSize;
    }
    else {
        blockData->end = blockData->begin + QBOX_RS_BLOCKSIZE;
    }

    blockData->ctx = NULL;
    blockData->cksum = NULL;

    bsize_len = sizeof(blockData->bsize_str);

    bzero(blockData->bsize_str, bsize_len);
    ret = snprintf(blockData->bsize_str, bsize_len, "%lu", blockData->end - blockData->begin);

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_MakeBlock(QBox_Client* self, QBox_RS_BlockSet* blockSet,
    FILE *fp, int blockIndex, QBox_RS_BlockData* blockData, QBox_Off_t chunkSize)
{
    QBox_Error err;
    QBox_Off_t rest = 0;
    QBox_Off_t step_size = 0;
    char* chunk = NULL;

    if (fp == NULL) {
        err.code = 400;
        err.message = "Invalid file handle";
        return err;
    }

    chunkSize = (chunkSize > 0) ? chunkSize : QBOX_RS_CHUNKSIZE;

    err = QBox_RS_InitBlockData(blockSet, blockData, blockIndex, 0);

    rest = blockData->end - blockData->current;
    step_size = (rest > chunkSize) ? chunkSize : rest;
    chunk = malloc(step_size);

    fseek(fp, blockData->current, SEEK_SET);

    if (fread(chunk, step_size, 1, fp) == 0) {
        err.code = 400;
        err.message = "Failed in reading file";
        free(chunk);
        return err;
    }

    /* Put the first chunk */
    err = QBox_RS_PutChunk(self, chunk, step_size, 0, blockData);
    if (err.code != 200) {
        free(chunk);
        return err;
    }

    blockData->current += step_size;
    free(chunk);

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_PutBlock(QBox_Client* self, QBox_RS_BlockData* blockData, FILE *fp, QBox_Off_t chunkSize)
{
    QBox_Error err;
    QBox_Off_t rest = 0;
    QBox_Off_t step_size = 0;
    char* chunk = NULL;

    chunkSize = (chunkSize > 0) ? chunkSize : QBOX_RS_CHUNKSIZE;

    rest = blockData->end - blockData->current;

    if (rest > 0) {
        step_size = (rest > chunkSize) ? chunkSize : rest;
        chunk = malloc(step_size);

        fseek(fp, blockData->current, SEEK_SET);
        do {
            if (fread(chunk, step_size, 1, fp) == 0) {
                err.code = 400;
                err.message = "Failed in reading file";
                free(chunk);
                return err;
            }

            err = QBox_RS_PutChunk(self, chunk, step_size, blockData->current, blockData);
            if (err.code != 200) {
                free(chunk);
                return err;
            }

            rest -= step_size;
            blockData->current += step_size;
            step_size = (rest > chunkSize) ? chunkSize : rest;
        } while (rest > 0);

        free(chunk);
    } /* if */

    err.code = 200;
    err.message = "OK";
    return err;
}

QBox_Error QBox_RS_PutChunk(QBox_Client* self,
    char* chunk, QBox_Off_t size, QBox_Off_t offset,
    QBox_RS_BlockData* blockData)
{
    QBox_Error err;
	char* mimeEncoded = NULL;
	cJSON* root = NULL;
    char* url = NULL;
    char offsetStr[128];
    unsigned long chunk_crc32 = 0L;
    QBox_Int64 ret_crc32 = 0;

	mimeEncoded = QBox_String_Encode("application/octet-stream");

    if (blockData->ctx == NULL) {
        /* Make a new block and put the first chunk */
	    url = QBox_String_Concat(QBOX_UP_HOST, "/mkblk/", blockData->bsize_str, "/mime/", mimeEncoded, NULL);
    }
    else {
        /* Put other chunks */
        bzero(offsetStr, sizeof(offsetStr));
        QBox_snprintf(offsetStr, sizeof(offsetStr), "%lu", offset);

        url = QBox_String_Concat(QBOX_UP_HOST, "/bput/", blockData->ctx, "/", offsetStr, "/mime/", mimeEncoded, NULL);
    }

	err = QBox_Client_CallWithBuffer(self, &root, url, chunk, size);
	free(url);

	if (err.code == 200) {
		blockData->ctx = QBox_Json_GetString(root, "ctx", NULL);
		blockData->cksum = QBox_Json_GetString(root, "checksum", NULL);

		ret_crc32 = QBox_Json_GetInt64(root, "crc32", 0);
        chunk_crc32 = crc32(0L, NULL, 0);;
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
    const char* callbackParams, FILE* fp, QBox_Off_t chunkSize)
{
    QBox_Error err;
    QBox_RS_BlockSet blockSet;
    char* entry = NULL;
    int i = 0;

    err = QBox_RS_SplitFile(self, &blockSet, fp);
    if (err.code != 200) {
        QBox_RS_CleanupBlockSet(&blockSet);
        return err;
    }

    for (i = 0; i < blockSet.count; ++i) {
        err = QBox_RS_MakeBlock(self, &blockSet, fp, i, &blockSet.blocks[i], chunkSize);
        if (err.code != 200) {
            QBox_RS_CleanupBlockSet(&blockSet);
            return err;
        }

        err = QBox_RS_PutBlock(self, &blockSet.blocks[i], fp, chunkSize);
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
