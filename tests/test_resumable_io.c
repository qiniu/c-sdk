/*
 ============================================================================
 Name        : test_io_put.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"
#include "../qiniu/resumable_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

static const char bucket[] = "csdk";
static const char key[] = "key2";
static const char domain[] = "csdk.qiniudn.com";

static void clientIoPutFile(const char* uptoken)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, __FILE__, &extra);
	CU_ASSERT(err.code == 200);

	Qiniu_Log_Info("Qiniu_Rio_PutFile: %E", err);

	printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

	Qiniu_Client_Cleanup(&client);
}

static void notify(void* self, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret)
{
	Qiniu_Log_Info("nodify: %d, off: %d", blkIdx, ret->offset);
}

static void notifyErr(void* self, int blkIdx, int blkSize, Qiniu_Error err)
{
	Qiniu_Log_Warn("nodify: %d, err: %E", blkIdx, err);
}

static const Qiniu_Int64 testFsize = 4*1024 + 2;

static void clientIoPutBuffer(const char* uptoken)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;
	Qiniu_Seq seq;
	Qiniu_ReaderAt in;
	Qiniu_Int64 fsize = testFsize;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;
	extra.notify = notify;
	extra.notifyErr = notifyErr;
	extra.chunkSize = 1024;

	in = Qiniu_SeqReaderAt(&seq, (size_t)fsize, 10, '0', 0);

	err = Qiniu_Rio_Put(&client, &putRet, uptoken, key, in, fsize, &extra);

	printf("\n%s", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

	CU_ASSERT(err.code == 200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FoErrxvY99fW7npWmVii0RncWKme");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoGet(const char* url)
{
	Qiniu_Eq eq;
	Qiniu_Seq seq;
	Qiniu_Int64 fsize = testFsize;
	Qiniu_Reader in = Qiniu_SeqReader(&seq, (size_t)fsize, 10, '0', 0);
	Qiniu_Writer w = Qiniu_EqWriter(&eq, in);

	long code, httpCode;
	CURL* curl = curl_easy_init();
	Qiniu_Buffer respHeader;
	Qiniu_Buffer_Init(&respHeader, 1024);

	printf("url: %s\n", url);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, w.Write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, w.self);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Qiniu_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &respHeader);

	code = curl_easy_perform(curl);

	printf("\n%s", Qiniu_Buffer_CStr(&respHeader));
	printf("\ncurl.code: %d, eq.result: %d\n", (int)code, eq.result);
	CU_ASSERT(code == 0);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	CU_ASSERT(httpCode == 200);

	CU_ASSERT(Qiniu_Is(&eq));

	curl_easy_cleanup(curl);
}

void testResumableIoPut(void)
{
	Qiniu_Client client;
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_RS_GetPolicy getPolicy;
	char* uptoken;
	char* dnBaseUrl;
	char* dnRequest;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);

	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, NULL);

	Qiniu_RS_Delete(&client, bucket, key);
	clientIoPutFile(uptoken);

	Qiniu_RS_Delete(&client, bucket, key);
	clientIoPutBuffer(uptoken);

	Qiniu_Free(uptoken);

	Qiniu_Zero(getPolicy);
	dnBaseUrl = Qiniu_RS_MakeBaseUrl(domain, key);
	dnRequest = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, dnBaseUrl, NULL);

	clientIoGet(dnRequest);

	Qiniu_Free(dnRequest);
	Qiniu_Free(dnBaseUrl);

	Qiniu_Client_Cleanup(&client);
}

