/*
 ============================================================================
 Name        : suites_rscli.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
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
#include "../qbox/rscli.h"
#include "../qbox/oauth2_passwd.h"
#include "test.h"

//#define TESTFILE "/home/wsy/文档/c-sdk/unit-test/test_file.txt"
#define TESTFILE "test_file.txt"
#define MESSAGE_LEVEL 1

QBox_Error err;
QBox_Client client;

void test_QBox_RSCli_PutFile(){
	QBox_RS_PutAuthRet putAuthRet;
	const char* tableName="test_rscli";
    QBox_RS_Create(&client,tableName);
	QBox_RS_PutAuth(&client, &putAuthRet);

	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, tableName, "rscli_file1", "application/octet-stream", TESTFILE, "", "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType=NULL customMeta=NULL
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, tableName, "rscli_file2", NULL, TESTFILE, NULL, "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType=NULL customMeta!=NULL & *customMeta!='\0'
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, tableName, "rscli_file3", NULL, TESTFILE, "test", "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch: resp!=NULL
    QBox_Buffer* buffer=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buffer,1024);
	err = QBox_RSCli_PutFile(buffer, putAuthRet.url, tableName, "rscli_file4", NULL, TESTFILE, "test", "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test Error
    //test err3
	err = QBox_RSCli_PutFile(NULL, NULL, tableName, "rscli_file5", NULL, TESTFILE, "test", "key=rs_demo.c");
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,3);


    QBox_RS_StatRet* ret=malloc(sizeof(QBox_RS_StatRet));
    err=QBox_RS_Stat(&client,ret,tableName,"rscli_file1");
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,"FqEDbBSj-G-XfRKU4fAZu6vUoSIr");
    CU_ASSERT_STRING_EQUAL(ret->mimeType,"application/octet-stream");
    CU_ASSERT_EQUAL(ret->fsize,29);

	//QBox_RS_Drop(&client, tableName);
}

void test_QBox_RSCli_PutStream(){
	QBox_RS_PutAuthRet putAuthRet;
	const char* tableName="test_rscli";
    QBox_RS_Create(&client,tableName);
	QBox_RS_PutAuth(&client, &putAuthRet);

    char* pStream = NULL;
    int bytes=4;
    pStream = malloc(bytes);
    strcpy(pStream,"1234");
    //test branch:mimeType!=NULL customMeta!=NULL
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rscli_stream1.c","application/octet-stream",pStream,bytes,"test","key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType!=NULL customMeta="\0"
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rscli_stream2.c","application/octet-stream",pStream,bytes,"","key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType!=NULL customMeta=NULL
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rscli_stream3.c","application/octet-stream",pStream,bytes,NULL,"key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType=NULL customMeta=NULL
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rscli_stream4.c",NULL,pStream,bytes,NULL,"key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    free(pStream);

    QBox_RS_StatRet* ret=malloc(sizeof(QBox_RS_StatRet));
    err=QBox_RS_Stat(&client,ret,tableName,"rscli_stream1.c");
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,"FnEQ7aTQngYqpeSjkLClcqwNLAIg");
    CU_ASSERT_STRING_EQUAL(ret->mimeType,"application/octet-stream");
    CU_ASSERT_EQUAL(ret->fsize,4);

	//QBox_RS_Drop(&client, tableName);
}

void test_QBox_RSCli_UploadStream(){
	QBox_RS_PutAuthRet putAuthRet;
	const char* tableName="test_rscli";
	const char* key="rscli1";
    QBox_RS_Create(&client,tableName);
	QBox_RS_PutAuth(&client, &putAuthRet);

	const char* uptoken="";
    QBox_RS_PutPolicy* auth=malloc(sizeof(QBox_RS_PutPolicy));
    auth->scope="test_rscli";
    auth->callbackUrl=0;
    auth->callbackBodyType=0;
    auth->customer=0;
    auth->asyncOps=0;
    auth->returnBody=0;
    auth->expires=3600;              // 可选。默认是 3600 秒
    auth->escape=0;              // 可选。非 0 表示 Callback 的 Params 支持转义符
    auth->detectMime=0;         // 可选。默认是 3600 秒
    uptoken=QBox_RS_PutPolicy_Token(auth);

    char* pStream = NULL;
    int bytes=4;
    pStream = malloc(bytes);
    strcpy(pStream,"1234");
    //test branch:mimeType!=NULL customMeta!=NULL
	err=QBox_RSCli_UploadStream(NULL,tableName,key,"application/octet-stream",pStream,bytes,"test","key=rs_demo.c",uptoken);
    CU_ASSERT_EQUAL(err.code,200);
	err=QBox_RSCli_UploadStream(NULL,tableName,"rscli2","application/octet-stream",pStream,bytes,"","key=rs_demo.c",uptoken);
    CU_ASSERT_EQUAL(err.code,200);
	err=QBox_RSCli_UploadStream(NULL,tableName,"rscli3","application/octet-stream",pStream,bytes,NULL,"key=rs_demo.c",uptoken);
    CU_ASSERT_EQUAL(err.code,200);
	err=QBox_RSCli_UploadStream(NULL,tableName,"rscli4",NULL,pStream,bytes,"test","key=rs_demo.c",uptoken);
    CU_ASSERT_EQUAL(err.code,200);

    QBox_RS_StatRet* ret=malloc(sizeof(QBox_RS_StatRet));
    err=QBox_RS_Stat(&client,ret,tableName,key);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,"FnEQ7aTQngYqpeSjkLClcqwNLAIg");
    CU_ASSERT_STRING_EQUAL(ret->mimeType,"application/octet-stream");
    CU_ASSERT_EQUAL(ret->fsize,4);

	//QBox_RS_Drop(&client, tableName);

}

/**//*---- test suites ------------------*/
int suite_init_rscli(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";
	QBox_Zero(client);
	QBox_Global_Init(-1);
	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_rcli(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(rscli)
QBOX_TEST(test_QBox_RSCli_PutFile)
QBOX_TEST(test_QBox_RSCli_PutStream)
QBOX_TEST(test_QBox_RSCli_UploadStream)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(rscli,suite_init_rscli,suite_clean_rcli)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/
void AddTestsRsCli(void)
{
        QBOX_TEST_REGISTE(rscli)
}

