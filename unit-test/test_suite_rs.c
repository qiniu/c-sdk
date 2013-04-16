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
#include "../qbox/rs.h"
#include "../qbox/base.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE1 "/home/wsy/文档/SDKUnitTest/src/test_file.txt"
//#define TESTFILE2 "/home/wsy/文档/SDKUnitTest/src/test_file2.txt"
#define TESTFILE1 "test_file.txt"
#define TESTFILE2 "test_file2.txt"

void test_QBox_RS_PutAuth(){
    QBox_RS_PutAuthRet* ret=malloc(sizeof(QBox_RS_PutAuthRet));

	//test branch: err.code!=200
	QBOX_ACCESS_KEY = "err.code!=200 branch";
    err=QBox_RS_PutAuth(&client,ret);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,401);
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";

	//test branch: err.code=200
    err=QBox_RS_PutAuth(&client,ret);
	CU_ASSERT_EQUAL(err.code,200);
	//printf("ret\n url=%s\n expiresIn=%ld\n",ret->url,(long)ret->expiresIn);
    free(ret);
}

void test_QBox_RS_PutAuthEx(){
    int expiresIn=7200;
    QBox_RS_PutAuthRet* ret=malloc(sizeof(QBox_RS_PutAuthRet));

    //test callbackUrl=NULL
    err=QBox_RS_PutAuthEx(&client,ret,NULL,expiresIn);
	CU_ASSERT_EQUAL(err.code,200);
	//printf("\n\nret\n url=%s\n expiresIn=%ld\n",ret->url,(long)ret->expiresIn);
    free(ret);

	//test callbackUrl="\0"
	ret=malloc(sizeof(QBox_RS_PutAuthRet));
    err=QBox_RS_PutAuthEx(&client,ret,"\0",expiresIn);
	CU_ASSERT_EQUAL(err.code,200);
	//printf("\n\nret\n url=%s\n expiresIn=%ld\n",ret->url,(long)ret->expiresIn);
    free(ret);

	//test callbackUrl!=NULL &&callbackUrl!="\0"
	///err400 请求参数错误 why?
    const char* callbackUrl="www.justTest.com";
    ret=malloc(sizeof(QBox_RS_PutAuthRet));
    err=QBox_RS_PutAuthEx(&client,ret,callbackUrl,expiresIn);
	CU_ASSERT_EQUAL(err.code,400);
	//printf("\n\nret\n url=%s\n expiresIn=%ld\n",ret->url,(long)ret->expiresIn);
	//printf("\n%d\n",err.code);
    free(ret);


	//test branch: err.code!=200
	QBOX_ACCESS_KEY = "err.code!=200 branch";
	ret=malloc(sizeof(QBox_RS_PutAuthRet));
    err=QBox_RS_PutAuthEx(&client,ret,NULL,expiresIn);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,401);
	//printf("\n\nret\n url=%s\n expiresIn=%ld\n",ret->url,(long)ret->expiresIn);
    free(ret);
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
}

void test_QBox_RS_Put(){
    const char* tableName="c_test_table_11";
    const char* key="c_test_key_1";
    QBox_RS_PutRet* ret=malloc(sizeof(QBox_RS_PutRet));
    const char* mimeType="application/octet-stream";
    FILE* file;
    QBox_Reader source;
    QBox_Int64 size=10;
    const char* customMeta="JUST FOR TEST";
    //*
    //Test
    //test branch: err!=200(=631).put a file into the non-existent table.
    QBox_RS_Drop(&client,tableName);
    file=fopen(TESTFILE1,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
	QBox_RS_Drop(&client, tableName);
	QBox_RS_Delete(&client,tableName,key);
    err=QBox_RS_Put(&client,ret,tableName,key,NULL,source,size,NULL);
    //CU_ASSERT_EQUAL(err.code,200);
    //printf("\n\n%d\n",err.code);
    CU_ASSERT_EQUAL(err.code,599);
    //printf("\n%d\n%s\n",err.code,err.message);
    //*/
    //*
    //test branch: PmimeType=NULL.customMeta=NULL
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,NULL,source,size,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);
    //*/
    //test branch: mimeType="cotet-stream".customMeta=NULL
    //*
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,mimeType,source,size,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);
    //*/
    //*
    //test branch: mimeType="cotet-stream".customMeta!=NULL
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,mimeType,source,size,customMeta);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);
    //*/
    //*
    //test branch: mimeType="cotet-stream".customMeta!=NULL
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,mimeType,source,size,"\0");
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);
    //*/
    //*
	//test branch: err.code!=200
    QBox_RS_Create(&client, tableName);
	QBOX_ACCESS_KEY = "err.code!=200 branch";
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,NULL,source,size,NULL);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,401);
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
    QBox_RS_Drop(&client,tableName);
    //*/
    free(ret);
    //*/
}


void test_QBox_RS_PutFile(){
    const char* tableName="c_test_table_11";
    const char* key="c_test_key_1";
    QBox_RS_PutRet* ret=malloc(sizeof(QBox_RS_PutRet));
    const char* mimeType="application/octet-stream";
    const char* srcFile;
    const char* customMeta=NULL;
    QBox_RS_Create(&client, tableName);

    //test branch: fp==NULL expected err.code=-1
    srcFile="bazinga";
    err=QBox_RS_PutFile(&client,ret,tableName,key,mimeType,srcFile,customMeta);
    CU_ASSERT_EQUAL(err.code,-1);
    //test branch: fp!=NULL
    srcFile=TESTFILE1;
    err=QBox_RS_PutFile(&client,ret,tableName,key,mimeType,srcFile,customMeta);
    CU_ASSERT_EQUAL(err.code,200);

    QBox_RS_Drop(&client,tableName);
    free(ret);
}

void test_QBox_RS_Get(){
    QBox_RS_GetRet* ret=malloc(sizeof(QBox_RS_GetRet));
    const char* tableName="c_test_table_11";
    const char* key="c_test_key_1";
    const char* attName;
    //test branch: attName=NULL and err.code!=200
    attName=NULL;
    err=QBox_RS_Get(&client,ret,tableName,key,attName);
    //CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,400);
    //test branch:attName!=NULL and err.code=200
    attName="attName.txt";
    QBox_RS_Create(&client, tableName);
    QBox_RS_PutRet* putRet=malloc(sizeof(QBox_RS_PutRet));
    const char* srcFile=TESTFILE1;
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    if(err.code!=200){
        CU_ASSERT_EQUAL(err.code,200);}
    else{
        err=QBox_RS_Get(&client,ret,tableName,key,attName);
        CU_ASSERT_EQUAL(err.code,200);
    }
    free(putRet);
    QBox_RS_Drop(&client,tableName);
    //printf("\n  ret=\n    url=%s\n    hash=%s\n    mimetype=%s\n    size=%ld\n    expriseIn=%ld\n    ",ret->url,ret->hash,ret->mimeType,(long)ret->fsize,(long)ret->expiresIn);

    free(ret);
}

void test_QBox_RS_GetIfNotModified(){
    QBox_RS_GetRet* ret=malloc(sizeof(QBox_RS_GetRet));
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* attName;
    const char* base="";
    //test branch: attName=NULL and err.code!=200
    attName=NULL;
    err=QBox_RS_GetIfNotModified(&client,ret,tableName,"no",attName,base);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    //*
    //test branch:attName!=NULL and err.code=200
    attName="attName.txt";
    QBox_RS_Create(&client, tableName);
    QBox_RS_PutRet* putRet=malloc(sizeof(QBox_RS_PutRet));
    const char* srcFile=TESTFILE1;
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    if(err.code!=200){
        CU_ASSERT_EQUAL(err.code,200);}
    else{
        QBox_RS_Get(&client,ret,tableName,key,attName);
        base=(const char*)ret->hash;
        err=QBox_RS_GetIfNotModified(&client,ret,tableName,key,attName,base);
        //printf("\n\n%d\n%s\n",err.code,err.message);
        CU_ASSERT_EQUAL(err.code,200);
    }
    free(putRet);
    QBox_RS_Drop(&client,tableName);
    //printf("\n  ret=\n    url=%s\n    hash=%s\n    mimetype=%s\n    size=%ld\n    expriseIn=%ld\n    ",ret->url,ret->hash,ret->mimeType,(long)ret->fsize,(long)ret->expiresIn);
    //*/
    free(ret);
}

void test_QBox_RS_Stat(){
    QBox_RS_StatRet* ret=malloc(sizeof(QBox_RS_StatRet));
    const char* tableName="c_test_table_11";
    const char* key="c_test_key_1";
    //test branch:err!=200 expected err.code=631
    err=QBox_RS_Stat(&client,ret,tableName,key);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,631);

    //test branch:err!=200; expected err.code=612
    QBox_RS_Create(&client, tableName);

    err=QBox_RS_Stat(&client,ret,tableName,key);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,612);
    //test branch:err=200
    QBox_RS_PutRet* putRet=malloc(sizeof(QBox_RS_PutRet));
    const char* srcFile=TESTFILE2;
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    if(err.code!=200){
        CU_ASSERT_EQUAL(err.code,200);}
    else{
        err=QBox_RS_Stat(&client,ret,tableName,key);
        CU_ASSERT_EQUAL(err.code,200);
    }
    free(putRet);

    QBox_RS_Drop(&client,tableName);

    free(ret);
}

void test_QBox_RS_Publish(){
    const char* tableName="c_test_table_0";
    const char* domain="domain";

    err=QBox_RS_Publish(&client,tableName,domain);
    CU_ASSERT_NOT_EQUAL(err.code,200);
}

void test_QBox_RS_Unpublish(){
    const char* domain="domain";

    err=QBox_RS_Unpublish(&client,domain);
    CU_ASSERT_NOT_EQUAL(err.code,200);
}

void test_QBox_RS_Delete(){
    const char* tableName="c_test_table_11";
    const char* key="c_test_key_1";

    QBox_RS_Create(&client, tableName);
    QBox_RS_PutRet* putRet=malloc(sizeof(QBox_RS_PutRet));
    const char* srcFile=TESTFILE2;
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    if(err.code!=200){
        CU_ASSERT_EQUAL(err.code,200);}
    else{
        err=QBox_RS_Delete(&client,tableName,key);
        CU_ASSERT_EQUAL(err.code,200);
    }
    free(putRet);
    QBox_RS_Drop(&client,tableName);
}

void test_QBox_RS_Create_and_Drop(){
    const char* tableName="c-test1";
    //test to create a new table
	QBox_RS_Drop(&client, tableName);
	err=QBox_RS_Create(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);

    //test to create a table which has been created before
	err=QBox_RS_Create(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);
	/// is it ok to create a table after the table has been created ?

    //test to drop a table which has been created before
	err=QBox_RS_Drop(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);

    //test to drop a table which does't exist
	err=QBox_RS_Drop(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);
	/// is it ok to drop table after the table has been dropped ?

}

void test_QBox_RS_PutStream(){
    QBox_RS_PutRet* ret=malloc(sizeof(QBox_RS_PutRet));
    const char* tableName="c_test_table_22";
    const char* key="c_test_key_22";
    const char* mimeType="application/octet-stream";
    char* stream = NULL;
    int bytes=128;
    const char* customMeta="JUST FOR TEST";

	err=QBox_RS_Create(&client, tableName);

    stream = malloc(bytes);
    err=QBox_RS_PutStream(&client,ret,tableName,key,mimeType,stream,bytes,customMeta);
    CU_ASSERT_EQUAL(err.code,200);

    err=QBox_RS_PutStream(&client,ret,tableName,key,NULL,stream,bytes,NULL);
    CU_ASSERT_EQUAL(err.code,200);

    err=QBox_RS_PutStream(&client,ret,tableName,key,NULL,stream,bytes,"\0");
    CU_ASSERT_EQUAL(err.code,200);

    QBOX_ACCESS_KEY = "ERROR";
    err=QBox_RS_PutStream(&client,ret,tableName,key,NULL,stream,bytes,NULL);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";

	err=QBox_RS_Drop(&client, tableName);
    free(stream);
    free(ret);
}

//void test_QBox_RS_ResumablePut(){}

CU_TestInfo testcases_rs[] = {
        {"Testing QBox_RS_PutAuth:", test_QBox_RS_PutAuth},
        {"Testing QBox_RS_PutAuthEx:", test_QBox_RS_PutAuthEx},
        {"Testing QBox_RS_Put:", test_QBox_RS_Put},
        {"Testing QBox_RS_PutFile:", test_QBox_RS_PutFile},
        {"Testing QBox_RS_Get:", test_QBox_RS_Get},
        {"Testing QBox_RS_GetIfNotModified:", test_QBox_RS_GetIfNotModified},
        {"Testing QBox_RS_Stat:", test_QBox_RS_Stat},
        {"Testing QBox_RS_Publish:", test_QBox_RS_Publish},
        {"Testing QBox_RS_Unpublish:", test_QBox_RS_Unpublish},
        {"Testing QBox_RS_Delete:", test_QBox_RS_Delete},
        {"Testing QBox_RS_Create and QBox_RS_Drop:", test_QBox_RS_Create_and_Drop},
        {"Testing QBox_RS_PutStream:", test_QBox_RS_PutStream},
        CU_TEST_INFO_NULL
};


/**//*---- test suites ------------------*/
int suite_rs_init(void)
{
	//printf("error401 solution: Update QBOX_ACCESS_KEY & QBOX_SECRET_KEY in \"run_test.c\"。\n");
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_rs_clean(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

CU_SuiteInfo suites_rs[] = {
        {"Testing the qbox.rs:", suite_rs_init, suite_rs_clean, testcases_rs},
        CU_SUITE_INFO_NULL
};


/**//*---- setting enviroment -----------*/

void AddTestsRS(void)
{
        assert(NULL != CU_get_registry());
        assert(!CU_is_test_running());
        /**//* shortcut regitry */

        if(CUE_SUCCESS != CU_register_suites(suites_rs)){
                fprintf(stderr, "Register suites failed - %s ", CU_get_error_msg());
                exit(EXIT_FAILURE);
        }
}
