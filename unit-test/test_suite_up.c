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
#include "../qbox/up.h"
#include "../qbox/base.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"
#define TESTFILE "test_file.txt"

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


/**//*---- test suites ------------------*/
int suite_init_up(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);
}

int suite_clean_up(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(up)
//QBOX_TEST(test_QBox_UP_Mkblock)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(up,suite_init_up,suite_clean_up)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsUp(void)
{
        QBOX_TEST_REGISTE(up)
}
