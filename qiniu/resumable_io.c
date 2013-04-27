/*
 ============================================================================
 Name        : resumable_io.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "resumable_io.h"
#include "conf.h"
#include <curl/curl.h>
#include <sys/stat.h>

#define	blockBits	22
#define blockMask	((1 << blockBits) - 1)

/*============================================================================*/

static Qiniu_Error Qiniu_Rio_bput(
	Qiniu_Client* self, Qiniu_Rio_BlkputRet* ret, Qiniu_Reader body, int bodyLength, const char* url)
{
	Qiniu_Json* root;
	Qiniu_Error err = Qiniu_Client_CallWithBinary(self, &root, url, body, bodyLength, NULL);
	if (err.code == 200) {
		ret->ctx = Qiniu_Json_GetString(root, "ctx", NULL);
		ret->checksum = Qiniu_Json_GetString(root, "checksum", NULL);
		ret->host = Qiniu_Json_GetString(root, "host", NULL);
		ret->crc32 = Qiniu_Json_GetInt64(root, "crc32", 0);
		ret->offset = Qiniu_Json_GetInt64(root, "offset", 0);
		if (ret->ctx == NULL || ret->host == NULL || ret->offset == 0) {
			err.code = 9998;
			err.message = "unexcepted response: invalid ctx, host or offset";
		}
	}
	return err;
}

static Qiniu_Error Qiniu_Rio_Mkblock(
	Qiniu_Client* self, Qiniu_Rio_BlkputRet* ret, int blkSize, Qiniu_Reader body, int bodyLength)
{
	char* url = Qiniu_String_Format(128, "%s/mkblk/%d", QINIU_UP_HOST, blkSize);
	Qiniu_Error err = Qiniu_Rio_bput(self, ret, body, bodyLength, url);
	free(url);
	return err;
}

static Qiniu_Error Qiniu_Rio_Blockput(
	Qiniu_Client* self, Qiniu_Rio_BlkputRet* ret, Qiniu_Reader body, int bodyLength)
{
	char* url = Qiniu_String_Format(1024, "%s/bput/%s/%d", ret->host, ret->ctx, (int)ret->offset);
	Qiniu_Error err = Qiniu_Rio_bput(self, ret, body, bodyLength, url);
	free(url);
	return err;
}

/*============================================================================*/

static Qiniu_Error ErrUnmatchedChecksum = {
	Qiniu_Rio_UnmatchedChecksum, "unmatched checksum"
};

static Qiniu_Error Qiniu_Rio_ResumableBlockput(
	Qiniu_Client* c, Qiniu_Rio_BlkputRet* ret, Qiniu_ReaderAt f, int blkIdx, int blkSize, Qiniu_Rio_PutExtra* extra)
{
	Qiniu_Error err = {200, NULL};
	Qiniu_Tee tee;
	Qiniu_Section section;
	Qiniu_Reader body, body1;

	Qiniu_Crc32 crc32;
	Qiniu_Writer h = Qiniu_Crc32Writer(&crc32, 0);
	Qiniu_Int64 offbase = (Qiniu_Int64)(blkIdx) << blockBits;

	int chunkSize = extra->chunkSize;
	int bodyLength;
	int tryTimes;

	if (ret->ctx == NULL) {

		if (chunkSize < blkSize) {
			bodyLength = chunkSize;
		} else {
			bodyLength = blkSize;
		}

		body1 = Qiniu_SectionReader(&section, f, offbase, bodyLength);
		body = Qiniu_TeeReader(&tee, body1, h);

		err = Qiniu_Rio_Mkblock(c, ret, blkSize, body, bodyLength);
		if (err.code != 200) {
			return err;
		}
		if (ret->crc32 != crc32.val || (int)(ret->offset) != bodyLength) {
			return ErrUnmatchedChecksum;
		}
		extra->notify(extra->notifyRecvr, blkIdx, blkSize, ret);
	}

	while ((int)(ret->offset) < blkSize) {

		if (chunkSize < blkSize - (int)(ret->offset)) {
			bodyLength = chunkSize;
		} else {
			bodyLength = blkSize - (int)(ret->offset);
		}

		tryTimes = extra->tryTimes;

lzRetry:
		crc32.val = 0;
		body1 = Qiniu_SectionReader(&section, f, offbase + (Qiniu_Int64)(ret->offset), bodyLength);
		body = Qiniu_TeeReader(&tee, body1, h);

		err = Qiniu_Rio_Blockput(c, ret, body, bodyLength);
		if (err.code == 200) {
			if (ret->crc32 == crc32.val) {
				extra->notify(extra->notifyRecvr, blkIdx, blkSize, ret);
				continue;
			}
			Qiniu_Log_Warn("ResumableBlockput: invalid checksum, retry");
			err = ErrUnmatchedChecksum;
		} else {
			if (err.code == Qiniu_Rio_InvalidCtx) {
				ret->ctx = NULL; // reset
				Qiniu_Log_Warn("ResumableBlockput: invalid ctx, please retry");
				return err;
			}
			Qiniu_Log_WarnErr("ResumableBlockput: bput failed -", err);
		}
		if (tryTimes > 1) {
			tryTimes--;
			Qiniu_Log_Info("ResumableBlockput retrying ...");
			goto lzRetry;
		}
		break;
	}
	return err;
}

/*============================================================================*/
/* func Qiniu_Rio_PutXXX */

CURL* Qiniu_Client_reset(Qiniu_Client* self);
Qiniu_Error Qiniu_callex(CURL* curl, Qiniu_Buffer *resp, Qiniu_Json** ret, Qiniu_Bool simpleError, Qiniu_Buffer *resph);

Qiniu_Error Qiniu_Rio_Put(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, Qiniu_ReaderAt f, Qiniu_Int64 fsize, Qiniu_Rio_PutExtra* extra)
{
	Qiniu_Error err = {};
	return err;
}

Qiniu_Error Qiniu_Rio_PutFile(
	Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
	const char* uptoken, const char* key, const char* localFile, Qiniu_Rio_PutExtra* extra)
{
	Qiniu_FileInfo fi;
	Qiniu_File* f;
	Qiniu_Error err = Qiniu_File_Open(&f, localFile);
	if (err.code != 0) {
		return err;
	}
	err = Qiniu_File_Stat(f, &fi);
	if (err.code == 0) {
		err = Qiniu_Rio_Put(self, ret, uptoken, key, Qiniu_FileReaderAt(f), Qiniu_FileInfo_Fsize(fi), extra);
	}
	Qiniu_File_Close(f);
	return err;
}

