/*
 ============================================================================
 Name        : suites_io.c
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
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qiniu/io.h"
#include "../qiniu/rs.h"
#include "../qiniu/resumable_io.h"
#include "test.h"

#define TESTFILE_64B "test_file_64b.txt"
#define TESTFILE_128B "test_file_128b.txt"
static const char bucket[] = "Bucket";
static const char key[] = "key2";
static const char domain[] = "aatest.qiniudn.com";
static void test_Qiniu_Io_PutFile(){
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;
	char *uptoken;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

    getUptoken(&uptoken);
    deleteFile();

	err = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_EQUAL(err.code,200);
	CU_ASSERT_STRING_EQUAL(putRet.hash,"Fut-RpAh7-pO-hvTVPWcdLpZTQJN");

	err = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, TESTFILE_128B, &extra);
	CU_ASSERT_EQUAL(err.code,614);

	//printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Client_Cleanup(&client);
}

static void test_Qiniu_Io_PutBuffer(const char* uptoken)
{
	const char text[] = "Hello, world!";

	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Io_PutExtra extra;
	Qiniu_Io_PutRet putRet;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

    getUptoken(&uptoken);
    deleteFile();

	err = Qiniu_Io_PutBuffer(&client, &putRet, uptoken, key, text, sizeof(text)-1, &extra);
	CU_ASSERT_EQUAL(err.code,200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FpQ6cC0G80WZruH42o759ylgMdaZ");

	err = Qiniu_Io_PutBuffer(&client, &putRet, uptoken, key, text, sizeof(text)-1, &extra);
	CU_ASSERT_EQUAL(err.code,200);

	//printf("\n%s", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Client_Cleanup(&client);
}
static void test_Qiniu_Io_form_init(){
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;
	char *uptoken;

	Qiniu_Client_InitNoAuth(&client, 1024);
    //test mimeType!=NULL customMeta!=NULL callbackParams!=NULL
	Qiniu_Zero(extra);
	extra.bucket = bucket;
	extra.mimeType="application/octet-stream";
	extra.customMeta="test";
	extra.callbackParams="call.back.arams";

    getUptoken(&uptoken);
    deleteFile();

	err = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_EQUAL(err.code,200);

    //test customMeta="" callbackParams=""
	extra.customMeta="";
	extra.callbackParams="";

    deleteFile();
	err = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, TESTFILE_64B, &extra);

	Qiniu_Client_Cleanup(&client);
}
static void test_Qiniu_Io_call(){
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet *putRet=NULL;
	char *uptoken;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

    getUptoken(&uptoken);
    deleteFile();

	err = Qiniu_Io_PutFile(&client, putRet, uptoken, key, TESTFILE_64B, &extra);
	CU_ASSERT_EQUAL(err.code,200);
	CU_ASSERT_EQUAL(putRet,NULL);

    deleteFile();
	err = Qiniu_Io_PutFile(&client, &putRet, "", key, TESTFILE_64B, &extra);
	CU_ASSERT_EQUAL(err.code,400);

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

QINIU_TESTS_BEGIN(testcases_io)
QINIU_TEST(test_Qiniu_Io_PutFile)
QINIU_TEST(test_Qiniu_Io_PutBuffer)
QINIU_TEST(test_Qiniu_Io_form_init)
QINIU_TEST(test_Qiniu_Io_call)
QINIU_TESTS_END()

QINIU_SUITES_BEGIN()
QINIU_SUITE_EX(testcases_io,suite_init,suite_clean)
QINIU_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsIo(void)
{
        QINIU_TEST_REGISTE(io)
}

