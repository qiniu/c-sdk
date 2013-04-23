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
#include <curl/curl.h>
#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/TestDB.h>
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "../qbox/oauth2.h"
#include "../cJSON/cJSON.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"
#define TESTFILE "test_file.txt"


void test_QBox_Oauth2(){
    CU_ASSERT_EQUAL(QBox_Json_GetInt64(NULL,"test",16),16);
    int bodyLength=128;
    char* chkBuf = NULL;
    chkBuf = malloc(bodyLength);
    err=QBox_Client_CallWithBuffer(&client, NULL, "err", chkBuf, bodyLength);
    CU_ASSERT_EQUAL(err.code,400);
    err=QBox_Client_Call(&client, NULL, "err");
    CU_ASSERT_EQUAL(err.code,400);
    err=QBox_Client_CallNoRet(&client, "err");
    CU_ASSERT_EQUAL(err.code,400);

    free(chkBuf);
}

void test_QBox_Oauth2_digest(){
    char* url="http://iovip.qbox.me/file/pNHauxwbZ1kAXg2hxoQCe2HozeMRNAA_BQoAJz-MqLvy9aa6Voyou8YZWqmQlfISmuUx61aMqLtWjKi7Voyouzw_wMDAwD8-PykPf301Zq2UqOpKJDwOnO4cvvvapxfQBxgve1EKQq4NXPMaSzy3aXWEo-hzjQWUVrpF2x8FKvIS9eid9q-95rc4N9rLpQGYMc_kuD_NIwBcx9Px5-THxtMFGimK1qWAmkxy2CMYS1pHSxBPU15WUW2zl4Q=";

    QBox_Header* headers = NULL;

	CURL* curl = (CURL*)client.curl;
	curl_easy_reset(curl);
	QBox_Buffer_Reset(&client.b);
	if (client.root != NULL) {
		cJSON_Delete(client.root);
		client.root = NULL;
	}

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_URL, url);

    err=client.auth.itbl->Auth(client.auth.self,&headers, url, "test", 4);
    //printf("\n%s\n",*headers);
    CU_ASSERT_EQUAL(err.code,200);
    client.auth.self=malloc(64);
	QBox_Client_Cleanup(&client);
	free(client.auth.self);
	client.auth.self=NULL;
}

void test_QBox_Json_GetString(){
    QBox_Json json;
    json.next=NULL;
    json.type=cJSON_String;
    json.string=malloc(sizeof(char)*56);
    strcpy(json.string,"test");
    json.valuestring=malloc(sizeof(char)*56);
    strcpy(json.valuestring,"test_valuestring");

    QBox_Json jsonArray;
    jsonArray.child=&json;
    char* str=NULL;
    str=QBox_Json_GetString(&jsonArray,"test","not_find");
    CU_ASSERT_EQUAL(strcmp(str,"test_valuestring"),0);
    str=QBox_Json_GetString(&jsonArray,"test_not_find","not_find");
    CU_ASSERT_EQUAL(strcmp(str,"not_find"),0);
    json.type=cJSON_Number;
    str=QBox_Json_GetString(&jsonArray,"test","not_find");
    CU_ASSERT_EQUAL(strcmp(str,"not_find"),0);
}

void test_QBox_Json_GetInt64(){
    QBox_Json json;
    json.next=NULL;
    json.type=cJSON_Number;
    json.string=malloc(sizeof(char)*56);
    strcpy(json.string,"test");
    json.valuestring=malloc(sizeof(char)*56);
    strcpy(json.valuestring,"test_valuestring");
    json.valuedouble=333;

    QBox_Json jsonArray;
    jsonArray.child=&json;
    int rlt;
    rlt=QBox_Json_GetInt64(&jsonArray,"test",999);
    CU_ASSERT_EQUAL(rlt,333);
    rlt=QBox_Json_GetInt64(&jsonArray,"test_not_find",999);
    CU_ASSERT_EQUAL(rlt,999);
    json.type=cJSON_String;
    rlt=QBox_Json_GetInt64(&jsonArray,"test",999);
    CU_ASSERT_EQUAL(rlt,999);
}



/**//*---- test suites ------------------*/
int suite_init_oauth2(void)
{
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_oauth2(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(oauth2)
QBOX_TEST(test_QBox_Oauth2)
QBOX_TEST(test_QBox_Oauth2_digest)
QBOX_TEST(test_QBox_Json_GetString)
QBOX_TEST(test_QBox_Json_GetInt64)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(oauth2,suite_init_oauth2,suite_clean_oauth2)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsOauth2(void)
{
        QBOX_TEST_REGISTE(oauth2)
}
