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
#include "../../c-sdk-2.2.0/qbox/base.h"
#include "../../c-sdk-2.2.0/qbox/rs.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"


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

CU_TestInfo testcases_base[] = {
        {"Testing QBox_Buffer:", test_QBox_Buffer},
        {"Testing QBox_SectionReader:", test_QBox_SectionReader},
        CU_TEST_INFO_NULL
};


/**//*---- test suites ------------------*/
int suite_base_init(void)
{
	//printf("error401 solution: Update QBOX_ACCESS_KEY & QBOX_SECRET_KEY in \"run_test.c\"。\n");
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_base_clean(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

CU_SuiteInfo suites_base[] = {
        {"Testing the qbox.base:", suite_base_init, suite_base_clean, testcases_base},
        CU_SUITE_INFO_NULL
};


/**//*---- setting enviroment -----------*/

void AddTestsBase(void)
{
        assert(NULL != CU_get_registry());
        assert(!CU_is_test_running());
        /**//* shortcut regitry */

        if(CUE_SUCCESS != CU_register_suites(suites_base)){
                fprintf(stderr, "Register suites_base failed - %s ", CU_get_error_msg());
                exit(EXIT_FAILURE);
        }
}
