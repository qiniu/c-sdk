/*
 ============================================================================
 Name        : suites_rs.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
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
#include "test.h"
#include <openssl/hmac.h>
#include "../cJSON/cJSON.h"

#include <curl/curl.h>

QBox_Error err;
QBox_Client client;

//#define TESTFILE1 "/home/wsy/文档/c-sdk/unit-test/test_file.txt"
//#define TESTFILE2 "/home/wsy/文档/c-sdk/unit-test/test_file2.txt"
#define TESTFILE1 "test_file.txt"
#define TESTFILE2 "test_file2.txt"

void test_QBox_RS_PutAuth(){
    QBox_RS_PutAuthRet* ret=malloc(sizeof(QBox_RS_PutAuthRet));

	//test branch: err.code=200
    err=QBox_RS_PutAuth(&client,ret);
	CU_ASSERT_EQUAL(err.code,200);
	//printf("ret\n url=%s\n expiresIn=%ld\n",ret->url,(long)ret->expiresIn);

	//test branch: err.code!=200
	QBOX_ACCESS_KEY = "err401";
    err=QBox_RS_PutAuth(&client,ret);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,401);
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";

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
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    QBox_RS_PutRet* ret=malloc(sizeof(QBox_RS_PutRet));
    const char* mimeType="application/octet-stream";
    FILE* file;
    QBox_Reader source;
    QBox_Int64 size=10;
    const char* customMeta="JUST FOR TEST";

    //test branch: PmimeType=NULL.customMeta=NULL
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,NULL,source,size,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,"FthxcG3okD3aIkbvHVjdCdnHZ3QO");
    QBox_RS_Drop(&client,tableName);


    //test upload file with the same key
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,NULL,source,size,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,NULL,source,size,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,"FthxcG3okD3aIkbvHVjdCdnHZ3QO");
    QBox_RS_Drop(&client,tableName);

    //test branch: mimeType="cotet-stream".customMeta=NULL
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,mimeType,source,size,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);

    //test branch: mimeType="cotet-stream".customMeta!=NULL
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,mimeType,source,size,customMeta);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);

    //test branch: mimeType="cotet-stream".customMeta='\0'
    QBox_RS_Create(&client, tableName);
    file=fopen(TESTFILE1,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,mimeType,source,size,"\0");
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);

    //Test Error
    //test err631: put a file into the non-existent table.
    QBox_RS_Drop(&client,tableName);
    file=fopen(TESTFILE1,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    source=QBox_FILE_Reader(file);
    err=QBox_RS_Put(&client,ret,tableName,key,NULL,source,size,NULL);
    CU_ASSERT_EQUAL(err.code,599);

	//test err401
    QBox_RS_Create(&client, tableName);
	QBOX_ACCESS_KEY = "err401";
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
    QBox_RS_PutRet* ret=malloc(sizeof(QBox_RS_PutRet));
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* mimeType="application/octet-stream";
    const char* srcFile;
    const char* customMeta=NULL;

    QBox_RS_Create(&client, tableName);
    srcFile=TESTFILE1;
    err=QBox_RS_PutFile(&client,ret,tableName,key,mimeType,srcFile,customMeta);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,"FqEDbBSj-G-XfRKU4fAZu6vUoSIr");
    QBox_RS_Drop(&client,tableName);

    //test Error
    //test err-1:open source file failed
    QBox_RS_Create(&client, tableName);
    srcFile="bazinga";
    err=QBox_RS_PutFile(&client,ret,tableName,key,mimeType,srcFile,customMeta);
    CU_ASSERT_EQUAL(err.code,-1);
    QBox_RS_Drop(&client,tableName);
    free(ret);
}

void test_QBox_RS_PutStream(){
    QBox_RS_PutRet* putret=malloc(sizeof(QBox_RS_PutRet));
    QBox_RS_GetRet* getret;
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* mimeType="application/octet-stream";
    char* stream = NULL;
    int bytes=5;
    const char* customMeta="JUST FOR TEST";

	err=QBox_RS_Create(&client, tableName);

    stream = malloc(sizeof(char)*bytes);
    strcpy(stream,"test");
    err=QBox_RS_PutStream(&client,putret,tableName,key,mimeType,stream,bytes,customMeta);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(putret->hash,"FpYfpklYgY92dwcHJ1XXAY3NJ46U");
    getret=malloc(sizeof(QBox_RS_GetRet));
    err=QBox_RS_Get(&client,getret,tableName,key,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(getret->hash,"FpYfpklYgY92dwcHJ1XXAY3NJ46U");
    CU_ASSERT_STRING_EQUAL(getret->mimeType,"application/octet-stream");
    CU_ASSERT_EQUAL(getret->fsize,bytes);
    free(getret);

    err=QBox_RS_PutStream(&client,putret,tableName,key,NULL,stream,bytes,NULL);
    CU_ASSERT_EQUAL(err.code,200);

    err=QBox_RS_PutStream(&client,putret,tableName,key,NULL,stream,bytes,"\0");
    CU_ASSERT_EQUAL(err.code,200);
	err=QBox_RS_Drop(&client, tableName);
    //test Error
    //test err401
	err=QBox_RS_Create(&client, tableName);
    QBOX_ACCESS_KEY = "ERROR";
    err=QBox_RS_PutStream(&client,putret,tableName,key,NULL,stream,bytes,NULL);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,401);
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	//test err599
	err=QBox_RS_Drop(&client, tableName);
    err=QBox_RS_PutStream(&client,putret,tableName,key,NULL,stream,bytes,"\0");
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,599);
	QBox_RS_Drop(&client, tableName);

    free(stream);
    free(putret);
}

void test_QBox_RS_Get(){
    QBox_RS_GetRet* ret=malloc(sizeof(QBox_RS_GetRet));
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* attName="attName.txt";
    const char* srcFile=TESTFILE1;
    const char* hash=malloc(sizeof(char)*64);
    QBox_RS_PutRet* putRet;
    //test branch:attName!=NULL
    QBox_RS_Create(&client, tableName);
    putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    strcpy(hash,(const char*)putRet->hash);
    free(putRet);
    if(err.code!=200){CU_ASSERT_EQUAL(err.code,200);return;}
    err=QBox_RS_Get(&client,ret,tableName,key,attName);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,hash);
    CU_ASSERT_STRING_EQUAL(ret->mimeType,"application/octet-stream");
    QBox_RS_Drop(&client,tableName);

    //test branch:attName=NULL
    QBox_RS_Create(&client, tableName);
    putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    strcpy(hash,putRet->hash);
    free(putRet);
    if(err.code!=200){CU_ASSERT_EQUAL(err.code,200);return;}
    err=QBox_RS_Get(&client,ret,tableName,key,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(hash,ret->hash);
    QBox_RS_Drop(&client,tableName);

    //test err400
    QBox_RS_Drop(&client,tableName);
    err=QBox_RS_Get(&client,ret,tableName,key,NULL);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,400);
    //test err612
    QBox_RS_Create(&client, tableName);
    err=QBox_RS_Get(&client,ret,tableName,key,NULL);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,612);
    QBox_RS_Drop(&client,tableName);

    free(ret);
}

void test_QBox_RS_GetIfNotModified(){
    QBox_RS_GetRet* ret=malloc(sizeof(QBox_RS_GetRet));
    QBox_RS_PutRet* putRet;
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* attName="attName.txt";
    const char* base=malloc(sizeof(char)*64);
    const char* srcFile=TESTFILE1;

    //test branch:attName!=NULL
    QBox_RS_Create(&client, tableName);
    putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    if(err.code!=200){CU_ASSERT_EQUAL(err.code,200);return;}
    strcpy(base,(const char*)putRet->hash);
    free(putRet);
    err=QBox_RS_GetIfNotModified(&client,ret,tableName,key,attName,base);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,base);
    CU_ASSERT_STRING_EQUAL(ret->mimeType,"application/octet-stream");
    QBox_RS_Drop(&client,tableName);
    //test branch:attName=NULL
    QBox_RS_Create(&client, tableName);
    putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    if(err.code!=200){CU_ASSERT_EQUAL(err.code,200);return;}
    strcpy(base,(const char*)putRet->hash);
    free(putRet);
    err=QBox_RS_GetIfNotModified(&client,ret,tableName,key,NULL,base);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);
    //test Error
    //test err608
    QBox_RS_Create(&client, tableName);
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    if(err.code!=200){CU_ASSERT_EQUAL(err.code,200);return;}
    err=QBox_RS_GetIfNotModified(&client,ret,tableName,key,NULL,"err");
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,608);
    QBox_RS_Drop(&client,tableName);
    //test err400
    QBox_RS_Drop(&client,tableName);
    err=QBox_RS_GetIfNotModified(&client,ret,tableName,"no",NULL,"");
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,400);
    //test err612
    QBox_RS_Create(&client, tableName);
    err=QBox_RS_GetIfNotModified(&client,ret,tableName,key,NULL,"err");
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,612);
    QBox_RS_Drop(&client,tableName);

    free(ret);
}

void test_QBox_RS_Stat(){
    QBox_RS_StatRet* ret=malloc(sizeof(QBox_RS_StatRet));
    QBox_RS_PutRet* putRet;
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* srcFile=TESTFILE1;
    const char* hash=malloc(sizeof(char)*128);
    //test correct
    QBox_RS_Create(&client, tableName);
    putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    strcpy(hash,putRet->hash);
    free(putRet);
    if(err.code!=200){CU_ASSERT_EQUAL(err.code,200);return;}
    err=QBox_RS_Stat(&client,ret,tableName,key);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->hash,hash);
    CU_ASSERT_STRING_EQUAL(ret->mimeType,"application/octet-stream");
    QBox_RS_Drop(&client,tableName);

    //test err631
    QBox_RS_Drop(&client,tableName);
    err=QBox_RS_Stat(&client,ret,tableName,key);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,631);

    //test err612
    QBox_RS_Create(&client, tableName);
    err=QBox_RS_Stat(&client,ret,tableName,key);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,612);
    QBox_RS_Drop(&client,tableName);
    free(ret);
}

void test_QBox_RS_Publish(){
    QBox_RS_PutRet* putRet;
    const char* tableName="c_test_table_0";
    const char* domain="";
    const char* key="c_test_key_0";
    const char* srcFile=TESTFILE1;
    //test correct
    QBox_RS_Create(&client, tableName);
    putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    free(putRet);

    domain="iovip.qbox.me/Bucket";
    err=QBox_RS_Publish(&client,tableName,domain);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Unpublish(&client,domain);

    domain="iovip.qbox.me";
    err=QBox_RS_Publish(&client,tableName,domain);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,599);
    QBox_RS_Drop(&client,tableName);
}

void test_QBox_RS_Unpublish(){
    const char* domain;
    QBox_RS_PutRet* putRet;
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* srcFile=TESTFILE1;
    //test correct
    QBox_RS_Create(&client, tableName);
    putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    free(putRet);

    domain="iovip.qbox.me/Bucket";
    err=QBox_RS_Publish(&client,tableName,domain);
    CU_ASSERT_EQUAL(err.code,200);
    domain="iovip.qbox.me/Bucket";

    err=QBox_RS_Unpublish(&client,domain);
    CU_ASSERT_EQUAL(err.code,200);

    //test err599 document not found
    err=QBox_RS_Unpublish(&client,domain);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,599);
}

void test_QBox_RS_Delete(){
    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
    const char* srcFile=TESTFILE2;

    //test correct
    QBox_RS_Create(&client, tableName);
    QBox_RS_PutRet* putRet=malloc(sizeof(QBox_RS_PutRet));
    err=QBox_RS_PutFile(&client,putRet,tableName,key,NULL,srcFile,NULL);
    free(putRet);
    if(err.code!=200){CU_ASSERT_EQUAL(err.code,200);return;}
    err=QBox_RS_Delete(&client,tableName,key);
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,tableName);
    //test Error
    //test err631
    QBox_RS_Drop(&client,tableName);
    err=QBox_RS_Delete(&client,tableName,key);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,631);
    //test err631
    QBox_RS_Create(&client, tableName);
    err=QBox_RS_Delete(&client,tableName,key);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,612);
    QBox_RS_Drop(&client,tableName);
}

void test_QBox_RS_Create(){
    const char* tableName="c-test1";
    //test to create a new table
	QBox_RS_Drop(&client, tableName);
	err=QBox_RS_Create(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);
    //test to create a table which has been created before
	err=QBox_RS_Create(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);

    //testError
    //test err401
	QBOX_SECRET_KEY = "error";
	err=QBox_RS_Create(&client, tableName);
	CU_ASSERT_EQUAL(err.code,401);
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

}

void test_QBox_RS_Drop(){
    const char* tableName="c-test1";
    //test to drop a table which has been created before
	err=QBox_RS_Create(&client, tableName);
	err=QBox_RS_Drop(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);

    //test to drop a table which does't exist
	err=QBox_RS_Drop(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);

    //testError
    //test err401
	QBOX_SECRET_KEY = "error";
	err=QBox_RS_Drop(&client, tableName);
	CU_ASSERT_EQUAL(err.code,401);
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

}

void test_QBox_RS_PutPolicy_Token(){
    char* s,*s2,*ptr,*access_key,*encode_digest,*encoded_policy_str,*digest,*policy_str;
    QBox_RS_PutPolicy* auth=malloc(sizeof(QBox_RS_PutPolicy));
    auth->scope=0;
    auth->callbackUrl=0;
    auth->callbackBodyType=0;
    auth->customer=0;
    auth->asyncOps=0;
    auth->returnBody=0;
    auth->expires=0;              // 可选。默认是 3600 秒
    auth->escape=0;              // 可选。非 0 表示 Callback 的 Params 支持转义符
    auth->detectMime=0;         // 可选。非 0 表示在服务端自动检测文件内容的 MimeType
    s=QBox_RS_PutPolicy_Token(auth);

    auth->scope="c-test1";
    auth->callbackUrl="test.com";
    auth->callbackBodyType="test";
    auth->customer="test";
    auth->asyncOps="test";
    auth->returnBody="test";
    auth->expires=1800;           // 可选。默认是 3600 秒
    auth->escape=1;              // 可选。非 0 表示 Callback 的 Params 支持转义符
    auth->detectMime=1;         // 可选。非 0 表示在服务端自动检测文件内容的 MimeType
    s=QBox_RS_PutPolicy_Token(auth);
/*
    s2=s;
    ptr=strchr(s2,':');
    access_key=malloc(128);
    strncpy(access_key,s2,ptr-s2);
    s2=ptr+1;
    ptr=strchr(s2,':');
    encode_digest=malloc(128);
    strncpy(encode_digest,s2,ptr-s2);
    s2=ptr+1;
    encoded_policy_str=malloc(128);
    strcpy(encoded_policy_str,s2);
    printf("\n%s\n",access_key);
    CU_ASSERT_STRING_EQUAL(access_key,"cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z");
    policy_str=QBox_String_Decode(encoded_policy_str);
    */
}

void test_QBox_RS_GetPolicy_Token(){
    char* s;
    QBox_RS_GetPolicy* auth=malloc(sizeof(QBox_RS_GetPolicy));
    auth->expires=0;
    s=QBox_RS_GetPolicy_Token(auth);

    auth->scope="c-test1";
    auth->expires=1800;
    s=QBox_RS_GetPolicy_Token(auth);
}


/**//*---- test suites ------------------*/
int suite_init_rs(void)
{
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_rs(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(rs)
//*/rs.h
QBOX_TEST(test_QBox_RS_PutAuth)
QBOX_TEST(test_QBox_RS_PutAuthEx)
QBOX_TEST(test_QBox_RS_Put)
QBOX_TEST(test_QBox_RS_PutFile)
QBOX_TEST(test_QBox_RS_PutStream)
QBOX_TEST(test_QBox_RS_Get)
QBOX_TEST(test_QBox_RS_GetIfNotModified)
QBOX_TEST(test_QBox_RS_Stat)
QBOX_TEST(test_QBox_RS_Publish)
QBOX_TEST(test_QBox_RS_Unpublish)
QBOX_TEST(test_QBox_RS_Delete)
QBOX_TEST(test_QBox_RS_Create)
QBOX_TEST(test_QBox_RS_Drop)
//rs.h*/
//*rs_token.c/
QBOX_TEST(test_QBox_RS_PutPolicy_Token)
QBOX_TEST(test_QBox_RS_GetPolicy_Token)
//rs_token.c*/
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(rs,suite_init_rs,suite_clean_rs)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsRS(void)
{
        QBOX_TEST_REGISTE(rs)
}

