/*
 ============================================================================
 Name        : test.c
 Author      : Qiniu.com
 Version     : 1.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "../qbox/rs.h"
#include "../qbox/io.h"
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const char bucket[] = "a";
const char key[] = "key";

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

	printf("\n%s\n", QBox_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

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

	printf("\n%s", QBox_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

	CU_ASSERT(err.code == 200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FpQ6cC0G80WZruH42o759ylgMdaZ");

	QBox_Client_Cleanup(&client);
}

static void clientIoPut(const char* uptoken)
{
	const char text[] = "Hello, world!";

	QBox_Error err;
	QBox_Client client;
	QBox_BufReader bufr;
	QBox_Reader body;
	QBox_Io_PutExtra extra;
	QBox_Io_PutRet putRet;

	QBox_Client_InitNoAuth(&client, 1024);

	QBox_Zero(extra);
	extra.bucket = bucket;

	body = QBox_Buffer_Reader(&bufr, text, sizeof(text)-1);
	err = QBox_Io_Put(&client, &putRet, uptoken, key, body, bufr.limit, &extra);

	printf("\n%s", QBox_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

	CU_ASSERT(err.code == 200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FpQ6cC0G80WZruH42o759ylgMdaZ");

	QBox_Client_Cleanup(&client);
}

static void testIoPut()
{
	QBox_Client client;
	QBox_AuthPolicy putPolicy;
	char* uptoken;

	QBox_Client_Init(&client, 1024);

	QBox_Zero(putPolicy);
	putPolicy.scope = bucket;
	uptoken = QBox_MakeUpToken(&putPolicy);

	QBox_RS_Delete(&client, bucket, key);
	clientIoPut(uptoken);

/*	QBox_RS_Delete(&client, bucket, key);
	clientIoPutFile(uptoken);

	QBox_RS_Delete(&client, bucket, key);
	clientIoPutBuffer(uptoken);
*/
	free(uptoken);
	QBox_Client_Cleanup(&client);
}

QBOX_TESTS_BEGIN(qbox)
	QBOX_TEST(testIoPut)
QBOX_TESTS_END()

QBOX_ONE_SUITE(qbox)

int main()
{
	int err = 0;

	QBOX_ACCESS_KEY	= getenv("QINIU_ACCESS_KEY");
	QBOX_SECRET_KEY	= getenv("QINIU_SECRET_KEY");

	assert(QBOX_ACCESS_KEY != NULL);
	assert(QBOX_SECRET_KEY != NULL);

	QBox_Global_Init(-1);

	CU_initialize_registry();

	assert(NULL != CU_get_registry());
	assert(!CU_is_test_running());
	if (CU_register_suites(suites) != CUE_SUCCESS) {
		exit(EXIT_FAILURE);
	}

	CU_basic_set_mode(CU_BRM_NORMAL);
	CU_set_error_action(CUEA_FAIL);
	err = CU_basic_run_tests();
	CU_cleanup_registry();

	QBox_Global_Cleanup();
	return err;
}

