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
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"
#define TESTFILE "test_file.txt"


void test_QBox_Buffer(){
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buff,2);
    char* s="12345678901234567890";
    QBox_Buffer_Write(buff,s,2);
    //test branch self->curr == self->bufEnd in CStr()
    char* str=QBox_Buffer_CStr(buff);
    CU_ASSERT_EQUAL(strlen(str),2);

    //test branch snewSize < expandSize in expend()
    QBox_Buffer_Write(buff,s,16);
    str=QBox_Buffer_CStr(buff);
    CU_ASSERT_EQUAL(strlen(str),18);

    //test branch self->buf==NULL in Cleanup()
    free(buff->buf);
    buff->buf = NULL;
    QBox_Buffer_Cleanup(buff);
    CU_ASSERT_EQUAL(buff->buf,NULL);

    //test branch fd==-1 in QBox_FileReaderAt_Open
    CU_ASSERT_EQUAL(QBox_FileReaderAt_Open("err").self,NULL);
}

void test_QBox_SectionReader(){
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    QBox_Reader reader;
    QBox_ReaderAt readerAt;
    int result=-1;
    QBox_Buffer_Init(buff,256);
    readerAt=QBox_FileReaderAt_Open(TESTFILE);
    reader=QBox_SectionReader(readerAt,0,0);
    result=reader.Read(buff,0,0,reader.self);
    CU_ASSERT_EQUAL(result,0);
    reader=QBox_SectionReader(readerAt,0,100);
    result=reader.Read(buff,0,256,reader.self);
    CU_ASSERT_EQUAL(result,100);
}


/**//*---- test suites ------------------*/
int suite_init_base(void)
{
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_base(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(testcases_base)
QBOX_TEST(test_QBox_Buffer)
QBOX_TEST(test_QBox_SectionReader)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(testcases_base,suite_init_base,suite_clean_base)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsBase(void)
{
        QBOX_TEST_REGISTE(base)
}

