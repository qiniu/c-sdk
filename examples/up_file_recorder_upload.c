#include "../qiniu/resumable_io.h"
#include "../qiniu/http.h"
#include "../qiniu/rs.h"
#include "../qiniu/base.h"
#include "../cJSON/cJSON.h"
#include <openssl/md5.h>
#include <stdio.h>
#include <wchar.h>

#if defined(_WIN32)
#pragma comment(lib, "libeay32.lib")
#endif

#ifndef RS_FILE_RECORDER_UPLOAD
#define RS_FILE_RECORDER_UPLOAD
#pragma once
#include "../qiniu/http.h"
#include "../qiniu/resumable_io.h"

typedef struct _Qiniu_Rio_PutProgress_Recvr {
	const char *progressFilePath;
	Qiniu_Int64 fsize;
	int blkCnt;
	Qiniu_Rio_BlkputRet *blkputRets;
} Qiniu_Rio_PutProgress_Recvr;

void resumableUploadWithKey(Qiniu_Mac *mac, const char *bucket, const char *key, const char *localFile);
int resumableUploadNotify(void* recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret);
#endif

//七牛设计了自己的分片上传的机制并集成到sdk中，分片上传是实现断点续传的基础。
//分片上传的基本原理是把一个大文件按照4MB一个块进行切割，然后再把每个块再切割
//成较小的片，比如256KB，512KB，1MB，2MB，4MB。上传的基本单位就是片，当一个
//块里面的片上传完成之后，可以记录下这个块的相关信息，然后当所有块上传完成之后，
//再把所有块的信息上传，合并为一个文件。在这种机制下，我们可以把块的上传进度都
//写入本地临时文件中，如果中途发生文件上传中断，下次可以再从这个中断的地方继续上传。

Qiniu_Int64 getFileSzie(const char *localFile)
{
	Qiniu_FileInfo fi;
	Qiniu_Int64 fsize;
	Qiniu_Error error;
	Qiniu_File *f;

	error = Qiniu_File_Open(&f, localFile);
	if (error.code != 200)
	{
		fsize = 0;
	}
	else
	{
		error = Qiniu_File_Stat(f, &fi);
		if (error.code == 200) {
			fsize = Qiniu_FileInfo_Fsize(fi);
		}
	}

	return fsize;
}

char* getProgressFilePath(const char* bucket, const char* key,
	const char * localFilePath)
{
	MD5_CTX ctx;
	int dataLen = strlen(bucket) + strlen(key) + strlen(localFilePath) + 3;
	char *tmpData = (char*)malloc(sizeof(char)*dataLen);
	unsigned char md[16];
	char *buf = (char*)malloc(sizeof(char) * 42);
	memset(buf, 0, 33);
	char tmp[3] = { '\0' };
	int i = 0;
	sprintf(tmpData, "%s:%s:%s", bucket, key, localFilePath);
	tmpData[dataLen - 1] = '\0';

	MD5_Init(&ctx);
	MD5_Update(&ctx, tmpData, strlen(tmpData));
	MD5_Final(md, &ctx);
	for (i = 0; i < 16; i++)
	{
		sprintf(tmp, "%02x", md[i]);
		strcat(buf, tmp);
	}
	strcat(buf, ".progress");

	free(tmpData);

	return buf;
}

static int BLOCK_SIZE = 4 * 1024 * 1024;
static const int BLOCK_CONTEXT_LENGTH = 196;

char *resumableProgressMarshal(Qiniu_Rio_BlkputRet *putRets, int blockCnt)
{
	char *progress ;
	cJSON *root = cJSON_CreateArray();
	for (int i = 0; i < blockCnt; i++)
	{
		cJSON *item = cJSON_CreateObject();
		Qiniu_Rio_BlkputRet *blk = putRets+i;
		if (blk && blk->ctx)
		{
			cJSON_AddStringToObject(item, "host", blk->host);
			cJSON_AddStringToObject(item, "ctx", blk->ctx);
			cJSON_AddStringToObject(item, "checksum", blk->checksum);
			cJSON_AddNumberToObject(item, "crc32", blk->crc32);
			cJSON_AddNumberToObject(item, "offset", blk->offset);
		}
		else
		{
			cJSON_AddNullToObject(item, "host");
			cJSON_AddNullToObject(item, "ctx");
			cJSON_AddNullToObject(item, "checksum");
			cJSON_AddNumberToObject(item, "crc32", 0);
			cJSON_AddNumberToObject(item, "offset", 0);
		}

		cJSON_AddItemToArray(root, item);
	}

	progress = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	return progress;
}

int resumableUploadNotify(void* recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret)
{
	Qiniu_Rio_PutProgress_Recvr *pRecvr = (Qiniu_Rio_PutProgress_Recvr*)recvr;
	printf("Fsize: %lld, BlkIndex: %d, Offset: %d\n", pRecvr->fsize, blkIdx, ret->offset);
	
	if (pRecvr->progressFilePath)
	{
		if (ret->offset % BLOCK_SIZE == 0 || blkIdx==pRecvr->blkCnt-1)
		{
			printf("Write block %d progress\n", blkIdx);
			Qiniu_Rio_BlkputRet *blk = pRecvr->blkputRets+blkIdx;
			blk->checksum = strdup(ret->checksum);
			blk->crc32 = ret->crc32;
			blk->ctx = strdup(ret->ctx);
			blk->host = strdup(ret->host);
			blk->offset = ret->offset;

			FILE *progressRecordHandle = fopen(pRecvr->progressFilePath, "wb+");
			if (progressRecordHandle)
			{
				char *progress = resumableProgressMarshal(pRecvr->blkputRets, pRecvr->blkCnt);
				fwrite(progress, strlen(progress) + 1, 1, progressRecordHandle);
				fflush(progressRecordHandle);
				fclose(progressRecordHandle);
			}
		}
	}
	
	return QINIU_RIO_NOTIFY_OK;
}

int resumableUploadNotifyErr(void* recvr, int blkIdx, int blkSize, Qiniu_Error err)
{
	return QINIU_RIO_NOTIFY_OK;
}

void resumableUploadWithKey(Qiniu_Mac *mac, const char *bucket, const char *key, const char *localFile)
{
	//设置分片上传的参数
	Qiniu_Rio_Settings rioSettings;
	Qiniu_Zero(rioSettings);
	//可以设置为 256KB, 512KB, 1MB, 2MB, 4MB
	rioSettings.workers = 1;
	rioSettings.chunkSize = 4 * 1024 * 1024;
	Qiniu_Rio_SetSettings(&rioSettings);

	//实际情况下，从业务服务器获取，通过http请求
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	char *upToken = Qiniu_RS_PutPolicy_Token(&putPolicy, mac);

	Qiniu_Rio_PutProgress_Recvr putProgressRecvr;
	Qiniu_Error error;
	Qiniu_Rio_PutRet putRet;
	Qiniu_Rio_PutExtra putExtra;
	int blockCnt = 0;
	int blockIndex = 0;
	int progressFileLen = 0;
	int recordBlockCnt = 0;

	//初始化
	Qiniu_Client client;
	Qiniu_Global_Init(-1);
	Qiniu_Client_InitNoAuth(&client, 1024);
	Qiniu_Zero(putRet);
	Qiniu_Zero(putExtra);
	Qiniu_Zero(putProgressRecvr);
	Qiniu_Zero(error);

	//初始化进度状态
	putProgressRecvr.fsize = getFileSzie(localFile);
	blockCnt = Qiniu_Rio_BlockCount(putProgressRecvr.fsize);
	putProgressRecvr.blkCnt = blockCnt;
	putProgressRecvr.blkputRets = (Qiniu_Rio_BlkputRet*)malloc(sizeof(Qiniu_Rio_BlkputRet)*blockCnt);
	for (int i = 0; i < blockCnt; i++)
	{
		Qiniu_Zero(*(putProgressRecvr.blkputRets + i));
	}

	char *progressFilePath = getProgressFilePath(bucket, key, localFile);
	putProgressRecvr.progressFilePath = progressFilePath;

	printf("Local progress file is %s\n", progressFilePath);
	//尝试读取本地进度
	FILE *progressRecordHandle = fopen(progressFilePath, "rb+");
	if (progressRecordHandle)
	{
		fseek(progressRecordHandle, 0l, SEEK_END);
		progressFileLen = ftell(progressRecordHandle);
		char *progressBuffer = (char*)malloc(sizeof(char)*(progressFileLen + 1));
		//reset
		fseek(progressRecordHandle, 0L, SEEK_SET);
		while (!feof(progressRecordHandle))
		{
			fread(progressBuffer, sizeof(char), progressFileLen, progressRecordHandle);
		}
		fclose(progressRecordHandle);
		progressBuffer[progressFileLen] = '\0';

		cJSON *root = cJSON_Parse(progressBuffer);
		int ctxCnt = cJSON_GetArraySize(root);
		for (int i = 0; i < ctxCnt; i++)
		{
			cJSON *item = cJSON_GetArrayItem(root, i);
			Qiniu_Rio_BlkputRet bputRet;
			bputRet.ctx = strdup(Qiniu_Json_GetString(item, "ctx", NULL));
			bputRet.host = strdup(Qiniu_Json_GetString(item, "host", NULL));
			bputRet.checksum = strdup(Qiniu_Json_GetString(item, "checksum", NULL));
			bputRet.crc32 = Qiniu_Json_GetInt64(item, "crc32", 0);
			bputRet.offset = Qiniu_Json_GetInt64(item, "offset", 0);
			putProgressRecvr.blkputRets[i] = bputRet;
		}

		putExtra.progresses = putProgressRecvr.blkputRets;
		putExtra.blockCnt = ctxCnt;
		cJSON_Delete(root);
		free(progressBuffer);
	}

	//设置上传进度记录
	putExtra.notifyRecvr = &putProgressRecvr;
	putExtra.notify = resumableUploadNotify;
	putExtra.notifyErr = resumableUploadNotifyErr;

	//上传
	error = Qiniu_Rio_PutFile(&client, &putRet, upToken, key, localFile, &putExtra);
	if (error.code != 200)
	{
		if (error.code == 701)
		{
			//如果是701，说明进度文件过期了，删除之
			remove(progressFilePath);
		}
		printf("%d\n", error.code);
		printf("%s\n", error.message);
	}
	else
	{
		//上传成功之后删除进度文件
		remove(progressFilePath);
		printf("Key: %s\n", putRet.key);
		printf("Hash: %s\n", putRet.hash);
	}

	Qiniu_Client_Cleanup(&client);
	Qiniu_Global_Cleanup();
	Qiniu_Free(upToken);
	free(progressFilePath);
	free(putProgressRecvr.blkputRets);
}

int main() {
	char *accessKey = "<ak>";
    char *secretKey = "<sk>";
	char *bucket = "<bucket>";
	char *key = "<key>";
	char *localFile = "<filepath>";

	Qiniu_Mac mac;
	mac.accessKey = accessKey;
	mac.secretKey = secretKey;

	resumableUploadWithKey(&mac, bucket, key, localFile);

	return 0;
}
