 /*
 ============================================================================
 Name        : suites_base.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <curl/curl.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/io.h"
#include "../qbox/rs.h"
#include "test.h"


const char bucket[] = "a";
const char key[] = "key";
const char domain[] = "aatest.qiniudn.com";

static void clientIoPutFile(const char* uptoken)
{
	QBox_Error err;
	QBox_Client client;
	QBox_Io_PutExtra extra;
	QBox_Io_PutRet putRet;

	QBox_Client_InitNoAuth(&client, 1024);

	QBox_Zero(extra);
	extra.bucket = bucket;

	err = QBox_Io_PutFile(&client, &putRet, uptoken, key, __FILE__, &extra);
	CU_ASSERT(err.code == 200);

	//printf("\n%s\n", QBox_Buffer_CStr(&client.respHeader));
	//printf("hash: %s\n", putRet.hash);

	QBox_Client_Cleanup(&client);
}

static void clientIoPutBuffer(const char* uptoken)
{
	const char text[] = "Hello, world!";

	QBox_Error err;
	QBox_Client client;
	QBox_Io_PutExtra extra;
	QBox_Io_PutRet putRet;

	QBox_Client_InitNoAuth(&client, 1024);

	QBox_Zero(extra);
	extra.bucket = bucket;

	err = QBox_Io_PutBuffer(&client, &putRet, uptoken, key, text, sizeof(text)-1, &extra);

	//printf("\n%s", QBox_Buffer_CStr(&client.respHeader));
	//printf("hash: %s\n", putRet.hash);

	CU_ASSERT(err.code == 200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FpQ6cC0G80WZruH42o759ylgMdaZ");

	QBox_Client_Cleanup(&client);
}

static void clientIoGet(const char* dntoken)
{
	const char text[] = "Hello, world!";
	char* url = QBox_String_Concat("http://", domain, "/", key, "?token=", dntoken, NULL);

	long code;
	CURL* curl = curl_easy_init();
	QBox_Buffer resp;
	QBox_Buffer_Init(&resp, 1024);
	//printf("url: %s\n", url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBox_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
	code = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	CU_ASSERT(code == 0);
	printf("\n%s\n",QBox_Buffer_CStr(&resp));
	CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(&resp), text);
}

static void testIoPut()
{
	QBox_Error err;
	QBox_Client client;
	QBox_RS_PutPolicy putPolicy;
	QBox_RS_GetPolicy getPolicy;
	char* uptoken;
	char* dntoken;

	QBox_Client_Init(&client, 1024);
    QBox_RS_Create(&client,bucket);
	QBox_Zero(putPolicy);
	putPolicy.scope = bucket;
	uptoken = QBox_RS_PutPolicy_Token(&putPolicy);

	QBox_RS_Delete(&client, bucket, key);
	clientIoPutFile(uptoken);

	QBox_RS_Delete(&client, bucket, key);
	clientIoPutBuffer(uptoken);

	free(uptoken);

	QBox_Zero(getPolicy);
	getPolicy.scope = "*/*";
	dntoken = QBox_RS_GetPolicy_Token(&getPolicy);

	clientIoGet(dntoken);

	free(dntoken);

    //QBox_RS_Drop(&client,bucket);
	QBox_Client_Cleanup(&client);
}

/**//*---- test suites ------------------*/
int suite_init_io(void)
{
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";
	QBox_Global_Init(-1);

	return 0;
}

int suite_clean_io(void)
{
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(io)
QBOX_TEST(testIoPut)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(io,suite_init_io,suite_clean_io)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/
void AddTestsIo(void)
{
        QBOX_TEST_REGISTE(io)
}

