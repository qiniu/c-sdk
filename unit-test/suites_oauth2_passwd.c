/*
 ============================================================================
 Name        : qbox_test.h

 Author      : Wu Shi Yu
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <curl/curl.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "../qbox/oauth2_passwd.h"
#include "test.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"
#define TESTFILE "test_file.txt"


void test_QBox_Token_set(){
    QBox_Token* token=QBox_Token_New("test1","test2",3600);
    CU_ASSERT_EQUAL(strcmp(token->accessToken,"test1"),0);
    CU_ASSERT_EQUAL(strcmp(token->refreshToken,"test2"),0);
    CU_ASSERT_EQUAL(token->expiry,3600);

    QBox_Token_Release(token);
}

void test_QBox_Token_Release(){
    QBox_Token* token=QBox_Token_New("test1","test2",3600);
    free((void*)token->accessToken);
    token->accessToken=NULL;
    QBox_Token_Release(token);

}

void test_QBox_Token_ExchangeByPassword(){
    QBox_Token* token=NULL;
/*
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	CU_ASSERT_EQUAL(err.code,200);
	*/
	if(token==NULL)
        return;
    QBox_Token_Release(token);
}


void test_Token_Refresh(){
    QBox_Token* token=QBox_Token_New("accessToken","refreshToken",3600);

    err=QBox_Token_Refresh(token);
	CU_ASSERT_EQUAL(err.code,401);
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	CU_ASSERT_EQUAL(err.code,200);
    err=QBox_Token_Refresh(token);
	CU_ASSERT_EQUAL(err.code,200);

    QBox_Token_Release(token);
}

void test_QBox_Token_ExchangeByRefreshToken(){
    QBox_Token* token=NULL;
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	CU_ASSERT_EQUAL(err.code,200);
    err=QBox_Token_ExchangeByRefreshToken(&token,token->refreshToken);
	CU_ASSERT_EQUAL(err.code,200);
    err=QBox_Token_ExchangeByRefreshToken(&token,"refesh");
	CU_ASSERT_EQUAL(err.code,401);

    QBox_Token_Release(token);
}

void test_QBox_PasswordAuth(){
    QBox_Token* token=NULL;
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	CU_ASSERT_EQUAL(err.code,200);
    QBox_Client_InitByPassword(&client,token,1024);
    QBox_PasswordAuth* self = (QBox_PasswordAuth*)client.auth.self;
    self->token = NULL;
    self->authHeader = NULL;
	QBox_Client_Cleanup(&client);

	QBox_Zero(client);
	QBox_Global_Init(-1);
    QBox_Client_InitByPassword(&client,token,1024);
	QBox_Client_Cleanup(&client);

	QBox_Zero(client);
	QBox_Global_Init(-1);
}
void test_QBox_PasswordAuth_Auth(){

	QBox_RS_PutRet putRet;

	const char* tableName="test_Rs_demo_table_0";

    QBox_Token* token=NULL;
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	token=QBox_Token_New(token->accessToken,token->refreshToken,5);
	CU_ASSERT_EQUAL(err.code,200);
    QBox_Client_InitByPassword(&client,token,1024);
	QBox_Token_Release(token);
	QBox_RS_Delete(&client, "Bucket", "rs_demo.c");

    err = QBox_RS_Create(&client,tableName);
    CU_ASSERT_EQUAL(err.code,200);
	err = QBox_RS_PutFile(&client, &putRet, tableName, "rs/demo.c", "application/octet-stream", __FILE__, "");
	CU_ASSERT_EQUAL(err.code,200);
	if(err.code!=200)
	{
	    printf("\nQBox_RS_PutFile Erroe!\nerr %d:%s\n",err.code,err.message);
	}
	//printf("    QBox_RS_Drop\n");
	//err = QBox_RS_Drop(&client, tableName);
	//CU_ASSERT_EQUAL(err.code,200);

	QBox_Client_Cleanup(&client);
	QBox_Zero(client);
	QBox_Global_Init(-1);
}

void test_QBox_PasswordAuth_Auth_err(){

	const char* tableName="test_Rs_demo_table_0";

    QBox_Token* token=NULL;
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	token=QBox_Token_New(token->accessToken,token->refreshToken,5);
	CU_ASSERT_EQUAL(err.code,200);
    QBox_Client_InitByPassword(&client,token,1024);
	QBox_Token_Release(token);

    QBox_PasswordAuth* auth=(QBox_PasswordAuth*)(client.auth.self);
    strcpy(auth->token->refreshToken,"err");

	//err = QBox_RS_Drop(&client, tableName);

    CU_ASSERT_NOT_EQUAL(err.code,200);
	CU_ASSERT_EQUAL(err.code,401);

	QBox_Client_Cleanup(&client);

	QBox_Zero(client);
	QBox_Global_Init(-1);
}

/**//*---- test suites ------------------*/
int suite_init_oauth2_passwd(void)
{
	QBox_Zero(client);
	QBox_Global_Init(-1);

	return 0;
}

int suite_clean_oauth2_passwd(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(oauth2_passwd)
QBOX_TEST(test_QBox_Token_set)
QBOX_TEST(test_QBox_Token_Release)
QBOX_TEST(test_QBox_Token_ExchangeByPassword)
//QBOX_TEST(test_Token_Refresh)
//QBOX_TEST(test_QBox_Token_ExchangeByRefreshToken)
//QBOX_TEST(test_QBox_PasswordAuth)
//QBOX_TEST(test_QBox_PasswordAuth_Auth)
//QBOX_TEST(test_QBox_PasswordAuth_Auth_err)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(oauth2_passwd,suite_init_oauth2_passwd,suite_clean_oauth2_passwd)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsOauth2Passwd(void)
{
        QBOX_TEST_REGISTE(oauth2_passwd)
}
