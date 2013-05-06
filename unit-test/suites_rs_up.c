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
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "../qbox/up.h"
#include "../qbox/oauth2.h"
#include "test.h"

//#define TESTFILE_16M "/home/wsy/文档/SDKUnitTest/src/test_file_16M.txt"
//#define TESTFILE_1M "/home/wsy/文档/SDKUnitTest/src/test_file_1M .txt"

#define TESTFILE_16M "test_file_16M.txt"
#define TESTFILE_1M "test_file_1M.txt"

#define MESSAGE_LEVEL 0
//MESSAGE_LEVEL: used to set the level of output message
int testErr=200;
int block_notify_rs_up_err299(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    return 0;
}
QBox_Error up_demo(const char* fl,const char* customMeta){
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
    QBox_UP_FnBlockNotify blockNotify=NULL;

    if(testErr==299)
        blockNotify=block_notify_rs_up_err299;

    if(MESSAGE_LEVEL >= 1)
        printf("\nProcessing ... %s\n", fl);

    /* Delete old file */
    QBox_Zero(client2);
    QBox_Client_Init(&client2, 1024);
    if(MESSAGE_LEVEL >= 1)
        printf("QBox_RS_Delete\n");
    QBox_RS_Delete(&client2, "Bucket", fl);
    QBox_RS_Drop(&client2,"Bucket");

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
        if(MESSAGE_LEVEL >= 1)
            printf("QBox_RS_ResumablePut\n");

        QBox_RS_Create(&client2,"Bucket");
        entry = QBox_String_Concat("Bucket:", fl, NULL);
        err = QBox_RS_ResumablePut(
            &client,
            &putRet,
            prog,
            blockNotify, /* blockNotify    */
            NULL, /* chunkNotify    */
            NULL, /* notifyParams   */
            entry,
            "text/plain",
            f,
            fsize,
            customMeta, /* customMeta     */
            NULL  /* callbackParams */
        );
        free(entry);

        QBox_FileReaderAt_Close(f.self);

        if (err.code != 200) {
            if(MESSAGE_LEVEL >= 1)
                printf("QBox_RS_ResumablePut failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return err;
        }

        QBox_UP_Progress_Release(prog);

        /* Check uploaded file */
        if(MESSAGE_LEVEL >= 1)
            printf("QBox_RS_Get\n");

        err = QBox_RS_Get(&client2, &getRet, "Bucket", fl, NULL);

        if (err.code != 200) {
            if(MESSAGE_LEVEL >= 1)
                printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return err;
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
    return err;
}
void test_QBox_RS_ResumablePut(){
    QBox_Error err;
    err=up_demo(TESTFILE_16M,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    //test customMeta != NULL
    err=up_demo(TESTFILE_1M,"test");
    CU_ASSERT_EQUAL(err.code,200);
    //test err299
    testErr=299;
    err=up_demo(TESTFILE_16M,NULL);
    CU_ASSERT_EQUAL(err.code,299);
    testErr=200;
}

/**//*---- test suites ------------------*/
int suite_init_rs_up(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	return 0;
}

int suite_clean_rs_up(void)
{
	QBox_Global_Cleanup();
    return 0;
}

QBOX_TESTS_BEGIN(rs_up)
QBOX_TEST(test_QBox_RS_ResumablePut)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(rs_up,suite_init_rs_up,suite_clean_rs_up)
QBOX_SUITES_END()

/**//*---- setting enviroment -----------*/
void AddTestsRsUp(void)
{
        QBOX_TEST_REGISTE(rs_up)
}
