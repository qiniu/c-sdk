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
#include "..//qbox/up.h"
#include "..//qbox/base.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"

void test_QBox_UP_Mkblock(){
    QBox_UP_PutRet* ret=malloc(sizeof(QBox_UP_PutRet));
    int blkSize;
    QBox_Reader body;
    int bodyLength;

    FILE* file;
    file=fopen(TESTFILE,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    body=QBox_FILE_Reader(file);
    bodyLength=62196;
    blkSize=65536;
    err=QBox_UP_Mkblock(&client,ret,blkSize,body,bodyLength);
    CU_ASSERT_EQUAL(err.code,200);
    //printf("\n%d:%s\n",err.code,err.message);
    //printf("\nctx=%s\nchecksum=%s\ncrc32=%ld\n",ret->ctx,ret->checksum,(long)ret->crc32);
    /*
    QBOX_ACCESS_KEY = "test Err";
    file=fopen(TESTFILE,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    body=QBox_FILE_Reader(file);
    bodyLength=62196;
    blkSize=65536;
    err=QBox_UP_Mkblock(&client,ret,blkSize,body,bodyLength);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    printf("\n%d:%s\n",err.code,err.message);
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
    */
    //printf("\nctx=%s\nchecksum=%s\ncrc32=%ld\n",ret->ctx,ret->checksum,(long)ret->crc32);

    free(ret);
}
/*
void test_QBox_UP_Blockput(){
    QBox_UP_PutRet* ret=malloc(sizeof(QBox_UP_PutRet));
    int blkSize;
    char* ctx=malloc(sizeof(char)*128);
    int offset;
    QBox_Reader body;
    int bodyLength;

    FILE* file;
    file=fopen(TESTFILE,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    body=QBox_FILE_Reader(file);
    bodyLength=62196;
    blkSize=65536;
    err=QBox_UP_Mkblock(&client,ret,blkSize,body,bodyLength);
    CU_ASSERT_EQUAL(err.code,200);

    if(err.code!=200){free(ret);return ;}

    strcpy(ctx,ret->ctx);
    offset=0;
    err=QBox_UP_Blockput(&client,ret,ctx,offset,body,bodyLength);
    CU_ASSERT_EQUAL(err.code,200);
    printf("\n%d:%s\n",err.code,err.message);

    free(ret);
}
*/

CU_TestInfo testcases_up[] = {
        {"Testing QBox_UP_Mkblock:", test_QBox_UP_Mkblock},
        //{"Testing QBox_UP_Blockput:", test_QBox_UP_Blockput},

        CU_TEST_INFO_NULL
};


/**//*---- test suites ------------------*/
int suite_up_init(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_up_clean(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
    return 0;
}

CU_SuiteInfo suites_up[] = {
        {"Testing the qbox.up:", suite_up_init, suite_up_clean, testcases_up},
        CU_SUITE_INFO_NULL
};


/**//*---- setting enviroment -----------*/

void AddTestsUp(void)
{
        assert(NULL != CU_get_registry());
        assert(!CU_is_test_running());
        /**//* shortcut regitry */

        if(CUE_SUCCESS != CU_register_suites(suites_up)){
                fprintf(stderr, "Register suites qbox.up failed - %s ", CU_get_error_msg());
                exit(EXIT_FAILURE);
        }
}
