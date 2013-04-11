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
#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/TestDB.h>
#include "..//qbox/base.h"
#include "..//qbox/rs.h"
#include "..//qbox/oauth2_passwd.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"


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
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	CU_ASSERT_EQUAL(err.code,200);
    //test branches in function QBox_shouldEscape in the file base.c
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "Test  ~89");
	CU_ASSERT_NOT_EQUAL(err.code,200);

	err = QBox_Token_ExchangeByPassword(&token, "", "Test  ~89");
	CU_ASSERT_NOT_EQUAL(err.code,200);
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
    QBox_PasswordAuth* self = (QBox_PasswordAuth*)client.auth;
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
	//printf("    QBox_RS_Drop\n");
	err = QBox_RS_Drop(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);

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

    QBox_PasswordAuth* auth=(QBox_PasswordAuth*)(client.auth);
    strcpy(auth->token->refreshToken,"err");

	err = QBox_RS_Drop(&client, tableName);

    CU_ASSERT_NOT_EQUAL(err.code,200);
	CU_ASSERT_EQUAL(err.code,401);

	QBox_Client_Cleanup(&client);

	QBox_Zero(client);
	QBox_Global_Init(-1);
}

CU_TestInfo testcases_oauth2_passwd[] = {
        {"Testing QBox_Token_set:", test_QBox_Token_set},
        {"Testing QBox_Token_Release:", test_QBox_Token_Release},
        {"Testing QBox_Token_ExchangeByPassword:", test_QBox_Token_ExchangeByPassword},
        {"Testing QBox_Token_Refresh:", test_Token_Refresh},
        {"Testing QBox_Token_ExchangeByRefreshToken:", test_QBox_Token_ExchangeByRefreshToken},
        {"Testing QBox_PasswordAuth:", test_QBox_PasswordAuth},
        {"Testing QBox_PasswordAuth_Auth:", test_QBox_PasswordAuth_Auth},
        {"Testing QBox_PasswordAuth_Auth_err:", test_QBox_PasswordAuth_Auth_err},
        CU_TEST_INFO_NULL
};


/**//*---- test suites ------------------*/
int suite_oauth2_passwd_init(void)
{
	//printf("error401 solution: Update QBOX_ACCESS_KEY & QBOX_SECRET_KEY in \"run_test.c\"。\n");

	QBox_Zero(client);
	QBox_Global_Init(-1);


	return 0;
}

int suite_oauth2_passwd_clean(void)
{
	QBox_Client_Cleanup(&client);

	QBox_Global_Cleanup();

    return 0;
}

CU_SuiteInfo suites_oauth2_passwd[] = {
        {"Testing the qbox.oauth2_passwd:", suite_oauth2_passwd_init, suite_oauth2_passwd_clean, testcases_oauth2_passwd},
        CU_SUITE_INFO_NULL
};


/**//*---- setting enviroment -----------*/

void AddTestsOauth2Passwd(void)
{
        assert(NULL != CU_get_registry());
        assert(!CU_is_test_running());
        /**//* shortcut regitry */

        if(CUE_SUCCESS != CU_register_suites(suites_oauth2_passwd)){
                fprintf(stderr, "Register suites_oauth2 failed - %s ", CU_get_error_msg());
                exit(EXIT_FAILURE);
        }
}
