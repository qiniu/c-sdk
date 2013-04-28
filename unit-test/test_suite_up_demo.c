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
#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/TestDB.h>
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "../qbox/up.h"
#include "../qbox/oauth2.h"
#include "test.h"

//#define TESTFILE_16M "/home/wsy/文档/SDKUnitTest/src/test_file_16M.txt"
//#define TESTFILE_1M "/home/wsy/文档/SDKUnitTest/src/test_file_1M .txt"

#define TESTFILE_16M "test_file_16M.txt"
#define TESTFILE_1M "test_file_1M.txt"

#define NORMAL 0
#define TEST_NOT_NULL_OR_END 1
#define TEST_END 2

#define MESSAGE_LEVEL 0
//MESSAGE_LEVEL: used to set the level of output message

void up_demo(const char* fl,int testStyle){
    //const char* fl=TESTFILE_16M;
    QBox_Error err;
    QBox_Client client;
    QBox_Client client2;
    QBox_AuthPolicy auth;
    QBox_ReaderAt f;
    QBox_UP_PutRet putRet;
    QBox_RS_GetRet getRet;
    char* uptoken = NULL;
    char* entry = NULL;
    QBox_Json* root = NULL;
    QBox_UP_Progress* prog = NULL;
    QBox_Int64 fsize = 0;

    if(MESSAGE_LEVEL >= 1)
        printf("\nProcessing ... %s\n", fl);

    /* Delete old file */
    QBox_Zero(client2);
    QBox_Client_Init(&client2, 1024);
    if(MESSAGE_LEVEL >= 1)
        printf("QBox_RS_Delete\n");
    QBox_RS_Delete(&client2, "Bucket", fl);

    /* Upload file */
    QBox_Zero(client);
    QBox_Zero(auth);

    /* QBox_MakeUpToken() should be called on Biz-Server side */
    if(MESSAGE_LEVEL >= 1)
        printf("QBox_MakeUpToken\n");

	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
        if(MESSAGE_LEVEL >= 1)
            printf("Cannot generate UpToken!\n");
		return;
	}

    /* QBox_Client_InitByUpToken() and
     * other QBox_UP_xxx() functions should be called on Up-Client side */
    if(MESSAGE_LEVEL >= 1)
        printf("QBox_Client_InitByUpToken\n");

    QBox_Client_InitByUpToken(&client, uptoken, 1024);

    f = QBox_FileReaderAt_Open(fl);

    if ((int)f.self >= 0) {
        fsize = (QBox_Int64) lseek((int)f.self, 0, SEEK_END);
        if(MESSAGE_LEVEL >= 1)
            printf("fsize=%ld\n",(long)fsize);

        prog = QBox_UP_NewProgress(fsize);
        if(testStyle==TEST_NOT_NULL_OR_END){
            prog->progs[0].ctx=malloc(sizeof(char)*16);
            strcpy(prog->progs[0].ctx,"test");
        }
        else if(testStyle==TEST_END){
            prog->progs[0].ctx=malloc(sizeof(char)*16);
            strcpy(prog->progs[0].ctx,"end");
        }

        if(MESSAGE_LEVEL >= 1)
            printf("QBox_RS_ResumablePut\n");
        QBox_RS_Create(&client2,"Bucket");
        entry = QBox_String_Concat("Bucket:", fl, NULL);
        err = QBox_RS_ResumablePut(
            &client,
            &putRet,
            prog,
            NULL, /* blockNotify    */
            NULL, /* chunkNotify    */
            NULL, /* notifyParams   */
            entry,
            "text/plain",
            f,
            fsize,
            "test", /* customMeta     */
            NULL  /* callbackParams */
        );
        free(entry);

        QBox_FileReaderAt_Close(f.self);

        CU_ASSERT_EQUAL(err.code,200);

        if (err.code != 200) {
            if(MESSAGE_LEVEL >= 1)
                printf("QBox_RS_ResumablePut failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return;
        }

        QBox_UP_Progress_Release(prog);

        /* Check uploaded file */
        if(MESSAGE_LEVEL >= 1)
            printf("QBox_RS_Get\n");

        err = QBox_RS_Get(&client2, &getRet, "Bucket", fl, NULL);

        CU_ASSERT_EQUAL(err.code,200);

        if (err.code != 200) {
            if(MESSAGE_LEVEL >= 1)
                printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return;
        }

        CU_ASSERT_EQUAL(getRet.fsize,fsize);

        if(MESSAGE_LEVEL>=2)
            printf("Got url=[%s]\n", getRet.url);
        if(MESSAGE_LEVEL>=2)
            printf("Got fsize=%llu\n", getRet.fsize);

        QBox_RS_Delete(&client2, "Bucket", fl);
        QBox_Client_Cleanup(&client2);
    }

    QBox_Client_Cleanup(&client);
}
void test_by_up_demo(){
    up_demo(TESTFILE_16M,NORMAL);
}

void test_ctx(){
    up_demo(TESTFILE_1M,TEST_NOT_NULL_OR_END);
    up_demo(TESTFILE_1M,TEST_END);
}

void test_auth_policy(){
    QBox_AuthPolicy auth;
    auth.scope="test";
    auth.callbackUrl="test";
    auth.returnUrl="test";
    auth.expires=1800;
    char* uptoken = NULL;
	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
		printf("Cannot generate UpToken!\n");
		return;
	}
	///how to judge?
}


/**//*---- test suites ------------------*/
int suite_init_up_demo(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	return 0;
}

int suite_clean_up_demo(void)
{
	QBox_Global_Cleanup();
    return 0;
}

QBOX_TESTS_BEGIN(up_demo)
QBOX_TEST(test_by_up_demo)
QBOX_TEST(test_ctx)
QBOX_TEST(test_auth_policy)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(up_demo,suite_init_up_demo,suite_clean_up_demo)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsUpDemo(void)
{
        QBOX_TEST_REGISTE(up_demo)
}
