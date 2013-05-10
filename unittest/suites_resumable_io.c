/*
 ============================================================================
 Name        : suites_resumable_io.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <curl/curl.h>
 #include <openssl/hmac.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qiniu/resumable_io.h"
#include "../qiniu/rs.h"
#include "../qiniu/base.h"
#include "test.h"

#define defaultTryTimes         3
#define defaultWorkers          4
#define defaultChunkSize        (256 * 1024) // 256k

#define TESTFILE_64B "test_file_64b.txt"
#define TESTFILE_128B "test_file_128b.txt"
#define TESTFILE_256K "test_file_256k.txt"
#define TESTFILE_16M "test_file_16m.txt"
static const char bucket[] = "Bucket";
static const char key[] = "key2";
static const char domain[] = "aatest.qiniudn.com";

Qiniu_Client serverClient;
static void clientIoPutFile(const char* uptoken)
{
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

static void test_Qiniu_Rio_SetSettings(){
    Qiniu_Rio_Settings v;
    v.workers=0;
    v.taskQsize=0;
    v.chunkSize=0;
    v.tryTimes=0;
    v.threadModel.itbl=NULL;
    Qiniu_Rio_SetSettings(&v);
    v.workers=defaultWorkers;
    v.taskQsize=v.workers * 4;
    v.chunkSize=defaultChunkSize;
    v.tryTimes=defaultTryTimes;
    v.threadModel=Qiniu_Rio_ST;
    Qiniu_Rio_SetSettings(&v);
}

static void test_Qiniu_Rio_Put()
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;
	Qiniu_Seq seq;
	Qiniu_ReaderAt in;
	Qiniu_Int64 fsize = testFsize;
	char* uptoken;

	Qiniu_Client_InitNoAuth(&client, 1024);
	getUptoken(&uptoken);
	deleteFile();

	Qiniu_Zero(extra);
	extra.bucket = bucket;
	extra.notify = notify;
	extra.notifyErr = notifyErr;
	extra.chunkSize = 1024;

	in = Qiniu_SeqReaderAt(&seq, fsize, 10, '0', 0);

	err = Qiniu_Rio_Put(&client, &putRet, uptoken, key, in, fsize, &extra);
    /*
	printf("\n%s", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);
    */
	CU_ASSERT(err.code == 200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FoErrxvY99fW7npWmVii0RncWKme");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoGet(const char* dntoken)
{
	Qiniu_Eq eq;
	Qiniu_Seq seq;
	Qiniu_Int64 fsize = testFsize;
	Qiniu_Reader in = Qiniu_SeqReader(&seq, fsize, 10, '0', 0);
	Qiniu_Writer w = Qiniu_EqWriter(&eq, in);

	char* url = Qiniu_String_Concat("http://", domain, "/", key, "?token=", dntoken, NULL);

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

static void test_Qiniu_Rio_PutFile()
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;
	char *uptoken;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;
	extra.chunkSize=64 * 1024;

    getUptoken(&uptoken);
    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_EQUAL(err.code,200);
	CU_ASSERT_STRING_EQUAL(putRet.hash,"Fut-RpAh7-pO-hvTVPWcdLpZTQJN");

	extra.chunkSize=64 * 1024;
    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_256K, &extra);
	CU_ASSERT_EQUAL(err.code,200);
	CU_ASSERT_STRING_EQUAL(putRet.hash,"FsLBNbB5-bYoF--rBdKldsUoLSEp");
	//printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));

	extra.chunkSize=256 * 1024;
    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_16M, &extra);
	CU_ASSERT_EQUAL(err.code,200);
	CU_ASSERT_STRING_EQUAL(putRet.hash,"lupYG7WWC1EiG8NhlKo415Pr9gQg");
    //test Error
    //test error:upload an unexisted file
	extra.bucket = bucket;
    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, "Bazinga", &extra);
	CU_ASSERT_NOT_EQUAL(err.code,200);

    //test error:unexisted bucket
	extra.bucket = "Bazinga";
    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_NOT_EQUAL(err.code,200);
    //test error:upload different files with the same key
	extra.bucket = bucket;
    deleteFile();
    Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_128B, &extra);
	CU_ASSERT_EQUAL(err.code,614);

	//printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));

	Qiniu_Client_Cleanup(&client);
}

static void notifyEmpty(void* self, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret)
{
}

static void notifyErrEmpty(void* self, int blkIdx, int blkSize, Qiniu_Error err)
{
}
static void test_Qiniu_Rio_PutExtra_Init()
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;
	char *uptoken;

	Qiniu_Client_InitNoAuth(&client, 1024);
    getUptoken(&uptoken);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_EQUAL(err.code,200);

	extra.chunkSize=32;
	extra.tryTimes=defaultTryTimes;
	extra.notify=notifyEmpty;
	extra.notifyErr=notifyErrEmpty;
	extra.threadModel=Qiniu_Rio_ST;

    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_EQUAL(err.code,200);

    //test Error
    //test blockCnt != blockCnt
	extra.chunkSize=32;
	extra.blockCnt=2;
	extra.progresses=(Qiniu_Rio_BlkputRet*)malloc(sizeof(Qiniu_Rio_BlkputRet) * extra.blockCnt);

    deleteFile();
	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_NOT_EQUAL(err.code,200);
	CU_ASSERT_EQUAL(err.code,Qiniu_Rio_InvalidPutProgress);

	Qiniu_Client_Cleanup(&client);
}


/**//*---- test suites ------------------*/
static int suite_init(void)
{

	QINIU_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QINIU_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	Qiniu_Global_Init(-1);

	Qiniu_Client_InitMacAuth(&serverClient, 1024, NULL);
	return 0;
}

static int suite_clean(void)
{
	Qiniu_Client_Cleanup(&serverClient);
	Qiniu_Global_Cleanup();

    return 0;
}

QINIU_TESTS_BEGIN(testcases_resumable_io)
QINIU_TEST(test_Qiniu_Rio_SetSettings)
QINIU_TEST(test_Qiniu_Rio_Put)
QINIU_TEST(test_Qiniu_Rio_PutFile)
QINIU_TEST(test_Qiniu_Rio_PutExtra_Init)
QINIU_TESTS_END()

QINIU_SUITES_BEGIN()
QINIU_SUITE_EX(testcases_resumable_io,suite_init,suite_clean)
QINIU_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsResumableIo(void)
{
        QINIU_TEST_REGISTE(resumable_io)
}


