/*
 ============================================================================
 Name        : resumable_io.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : RS
 ============================================================================
 */

#include "resumable_io.h"
#include "conf.h"
#include <curl/curl.h>
#include <sys/stat.h>

/*============================================================================*/

typedef struct _Qiniu_Rio_BlkputRet {
	const char* ctx;
	const char* checksum;
	Qiniu_Uint32 crc32;
	Qiniu_Uint32 offset;
	const char* host;
} Qiniu_Rio_BlkputRet;

static Qiniu_Error Qiniu_Rio_bput(
	Qiniu_Client* self, Qiniu_Rio_BlkputRet* ret, Qiniu_Reader body, int bodyLength, const char* url)
{
	Qiniu_Json* root;
	Qiniu_Error err = Qiniu_Client_CallWithBinary(self, &root, url, body, Qiniu_Int64(bodyLength), NULL);
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
    Qiniu_Error err;
    char blkSizeStr[128];
    char* url = NULL;

    bzero(blkSizeStr, sizeof(blkSizeStr));
    Qiniu_snprintf(blkSizeStr, sizeof(blkSizeStr), "%d", blkSize);
	url = Qiniu_String_Concat(QBOX_UP_HOST, "/mkblk/", blkSizeStr, NULL);

    err = Qiniu_Rio_Chunkput(self, ret, body, bodyLength, url);
    free(url);

    return err;
}

Qiniu_Error Qiniu_Rio_Blockput(Qiniu_Client* self, Qiniu_Rio_PutRet* ret,
    const char* ctx, int offset, Qiniu_Reader body, int bodyLength)
{
    Qiniu_Error err;
    char offsetStr[128];
    char* url = NULL;

    bzero(offsetStr, sizeof(offsetStr));
    Qiniu_snprintf(offsetStr, sizeof(offsetStr), "%d", offset);

    url = Qiniu_String_Concat(QBOX_UP_HOST, "/bput/", ctx, "/", offsetStr, NULL);

    err = Qiniu_Rio_Chunkput(self, ret, body, bodyLength, url);
    free(url);

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

