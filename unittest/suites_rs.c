/*
 ============================================================================
 Name        : suites_rs.c
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
#include "../qiniu/rs.h"
#include "test.h"

static const char bucket[] = "Bucket";
static const char key[] = "key2";
static const char domain[] = "aatest.qiniudn.com";

static void test_Qiniu_RS_PutPolicy_Token(){
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_RS_PutPolicy putPolicy;
	char* uptoken;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);

	Qiniu_Zero(putPolicy);
	uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, NULL);
	printf("\n%s\n",uptoken);

	putPolicy.scope = bucket;
    putPolicy.callbackUrl="test";
    putPolicy.callbackBodyType="test";
    putPolicy.asyncOps="test";
    putPolicy.returnBody="test";
    putPolicy.customer="test";
    putPolicy.expires=1800;
    putPolicy.escape="test";
    putPolicy.detectMime="test";
	uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, NULL);
	printf("\n%s\n",uptoken);
}

static void test_Qiniu_RS_GetPolicy_Token(){
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_RS_GetPolicy getPolicy;
	char* dntoken;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);

	Qiniu_Zero(getPolicy);
	getPolicy.scope = "*/*";
	dntoken = Qiniu_RS_GetPolicy_Token(&getPolicy, NULL);
	printf("\n%s\n",dntoken);

    getPolicy.expires=1800;
    dntoken = Qiniu_RS_GetPolicy_Token(&getPolicy, NULL);
	printf("\n%s\n",dntoken);
}


static void test_Qiniu_RS_Stat(){
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_RS_StatRet ret;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);
    err=Qiniu_RS_Stat(&client,&ret,bucket,key);

    err=Qiniu_RS_Stat(&client,&ret,"can't find bucket",key);
}

static void test_Qiniu_RS_Delete(){
	Qiniu_Error err;
	Qiniu_Client client;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);

    err=Qiniu_RS_Delete(&client,"can't find bucket",key);
}
/**//*---- test suites ------------------*/
static int suite_init(void)
{

	QINIU_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QINIU_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	Qiniu_Global_Init(-1);


	return 0;
}

static int suite_clean(void)
{
	Qiniu_Global_Cleanup();

    return 0;
}

QINIU_TESTS_BEGIN(testcases_rs)
QINIU_TEST(test_Qiniu_RS_PutPolicy_Token)
QINIU_TEST(test_Qiniu_RS_GetPolicy_Token)
QINIU_TEST(test_Qiniu_RS_Stat)
QINIU_TEST(test_Qiniu_RS_Delete)
QINIU_TESTS_END()

QINIU_SUITES_BEGIN()
QINIU_SUITE_EX(testcases_rs,suite_init,suite_clean)
QINIU_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsRs(void)
{
        QINIU_TEST_REGISTE(rs)
}

