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
#include <openssl/hmac.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "../qbox/oauth2.h"
#include "../cJSON/cJSON.h"
#include "test.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE "/home/wsy/文档/SDKUnitTest/src/test_file.txt"
#define TESTFILE "test_file.txt"

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
    //test brach:sub != NULL  sub->type == cJSON_String
    str=QBox_Json_GetString(&jsonArray,"test","not_find");
    CU_ASSERT_STRING_EQUAL(str,"test_valuestring");
    //test brach:sub == NULL
    str=QBox_Json_GetString(&jsonArray,"test_not_find","not_find");
    CU_ASSERT_STRING_EQUAL(str,"not_find");
    //test brach:sub != NULL  sub->type != cJSON_String
    json.type=cJSON_Number;
    str=QBox_Json_GetString(&jsonArray,"test","not_find");
    CU_ASSERT_STRING_EQUAL(str,"not_find");
    //test brach:self == NULL
    str=QBox_Json_GetString(NULL,"test","not_find");
    CU_ASSERT_STRING_EQUAL(str,"not_find");
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
    //test branch:sub != NULL sub->type == cJSON_Number
    rlt=QBox_Json_GetInt64(&jsonArray,"test",999);
    CU_ASSERT_EQUAL(rlt,333);
    //test branch:sub == NULL
    rlt=QBox_Json_GetInt64(&jsonArray,"test_not_find",999);
    CU_ASSERT_EQUAL(rlt,999);
    //test branch:sub != NULL sub->type != cJSON_Number
    json.type=cJSON_String;
    rlt=QBox_Json_GetInt64(&jsonArray,"test",999);
    CU_ASSERT_EQUAL(rlt,999);
    //test brach:self == NULL
    rlt=QBox_Json_GetInt64(NULL,"test",999);
    CU_ASSERT_EQUAL(rlt,999);
}

static QBox_Error AuthForTest_Auth(void* self, QBox_Header** header, const char* url, const char* addition, size_t addlen)
{
}
static void AuthForTest_Release(void* self)
{
}
static QBox_Auth_Itbl Auth_Itbl_ForTest = {
	AuthForTest_Auth,
	AuthForTest_Release
};
static QBox_Auth AuthForTest = {
	"test",
	&Auth_Itbl_ForTest
};

void test_QBox_Client_InitEx(){
    QBox_Client* self=malloc(sizeof(QBox_Client));
    size_t bufSize=1024;
    QBox_Zero(*self);

    QBox_Client_InitEx(self,AuthForTest,bufSize);
    CU_ASSERT_STRING_EQUAL(self->curl,"");
    CU_ASSERT_EQUAL(self->root,NULL);
    CU_ASSERT_STRING_EQUAL(self->auth.self,"test");
    CU_ASSERT_EQUAL(self->b.bufEnd-self->b.buf,bufSize);
    CU_ASSERT_EQUAL(self->respHeader.bufEnd-self->respHeader.buf,bufSize);
}

void test_QBox_Client_InitNoAuth(){
    QBox_Client* self=malloc(sizeof(QBox_Client));
    size_t bufSize=1024;
    QBox_Zero(*self);

    QBox_Client_InitNoAuth(self,bufSize);

    CU_ASSERT_STRING_EQUAL(self->curl,"");
    CU_ASSERT_EQUAL(self->root,NULL);
    CU_ASSERT_EQUAL(self->auth.self,NULL);
    CU_ASSERT_EQUAL(self->auth.itbl,NULL);
    CU_ASSERT_EQUAL(self->b.bufEnd-self->b.buf,bufSize);
    CU_ASSERT_EQUAL(self->respHeader.bufEnd-self->respHeader.buf,bufSize);

}

void test_QBox_Client_Cleanup(){
    QBox_Client* self=malloc(sizeof(QBox_Client));
    size_t bufSize=1024;
    QBox_Zero(*self);
    QBox_Client_InitEx(self,AuthForTest,bufSize);
    self->root=malloc(sizeof(QBox_Json));
    self->root->next=NULL;
    self->root->type=cJSON_Number;
    self->root->string=NULL;
    self->root->valuestring=NULL;
    self->root->valuedouble=333;
    self->root->child=NULL;

    QBox_Client_Cleanup(self);
    CU_ASSERT_EQUAL(self->auth.itbl,NULL);
    CU_ASSERT_EQUAL(self->curl,NULL);
    CU_ASSERT_EQUAL(self->root,NULL);
    CU_ASSERT_EQUAL(self->b.buf,NULL);
    CU_ASSERT_EQUAL(self->respHeader.buf,NULL);
    /*
    //test brach:self->auth.itbl =self->curl =self->root= NULL
    QBox_Client_Cleanup(self);
    CU_ASSERT_EQUAL(self->auth.itbl,NULL);
    CU_ASSERT_EQUAL(self->curl,NULL);
    CU_ASSERT_EQUAL(self->root,NULL);
    CU_ASSERT_EQUAL(self->b.buf,NULL);
    CU_ASSERT_EQUAL(self->respHeader.buf,NULL);
    */
}


static QBox_Error TestCall_Auth(void* self, QBox_Header** header, const char* url, const char* addition, size_t addlen)
{
    QBox_Error error;
    if(strcmp(url,"err")==0){
        error.code=1234567;
        return error;
    }

	char const* path = NULL;
	char* auth = NULL;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int dgtlen = sizeof(digest);
	char* enc_digest = NULL;
	HMAC_CTX ctx;

	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();

	path = strstr(url, "://");
	if (path != NULL) {
		path = strchr(path + 3, '/');
	}
	if (path == NULL) {
		err.code = 400;
		err.message = "Invalid URL";
		return err;
	}

	/* Do digest calculation */
	HMAC_CTX_init(&ctx);

	HMAC_Init_ex(&ctx, QBOX_SECRET_KEY, strlen(QBOX_SECRET_KEY), EVP_sha1(), NULL);
	HMAC_Update(&ctx, path, strlen(path));
	HMAC_Update(&ctx, "\n", 1);

	if (addlen > 0) {
		HMAC_Update(&ctx, addition, addlen);
	}

	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

	enc_digest = QBox_Memory_Encode(digest, dgtlen);

	/* Set appopriate HTTP header */
	auth = QBox_String_Concat("Authorization: QBox ", QBOX_ACCESS_KEY, ":", enc_digest, NULL);
	free(enc_digest);

	*header = curl_slist_append(*header, auth);
	free(auth);

    error.code=200;
    return error;
}
static void TestCall_Release(void* self)
{
}
static QBox_Auth_Itbl TestCallAuth_Itbl = {
	TestCall_Auth,
	TestCall_Release
};
static QBox_Auth TestCallAuth = {
	"TestCall",
	&TestCallAuth_Itbl
};
void test_QBox_Client_CallWithBuffer(){
    QBox_Client* self=malloc(sizeof(QBox_Client));
    size_t bufSize=1024;
    QBox_Zero(*self);
    QBox_Client_InitEx(self,TestCallAuth,bufSize);


    QBox_Client* self2=malloc(sizeof(QBox_Client));
    QBox_Zero(*self2);
    QBox_Client_InitNoAuth(self2,bufSize);
	cJSON* root;

    const char* mimeType= "application/octet-stream";
	char* mimeEncoded = QBox_String_Encode(mimeType);
	char* entryURI = QBox_String_Concat3("c_test_table_0", ":", "test");
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url;
	url = QBox_String_Concat(QBOX_UP_HOST, "/rs-put/", entryURIEncoded, "/mime/", mimeEncoded, NULL);

    int bodyLength=128;
    char* chkBuf = NULL;
    //test self->auth.itbl != NULL err.code = 200
	QBox_RS_Create(&client,"c_test_table_0");
    chkBuf = malloc(bodyLength);
    err=QBox_Client_CallWithBuffer(self, &root, url, chkBuf, bodyLength);
    CU_ASSERT_EQUAL(err.code,200);
	//QBox_RS_Drop(&client,"c_test_table_0");

    //test self->auth.itbl != NULL err.code != 200
    chkBuf = malloc(bodyLength);
    err=QBox_Client_CallWithBuffer(self, root, "err", NULL, 0);
    CU_ASSERT_EQUAL(err.code,1234567);

    //test self->auth.itbl== NULL err
    chkBuf = malloc(bodyLength);
    err=QBox_Client_CallWithBuffer(self2, &root, url, chkBuf, bodyLength);
    CU_ASSERT_EQUAL(err.code,401);
}


void test_QBox_Client_CallWithBinary(){
    QBox_Client* self=malloc(sizeof(QBox_Client));
    size_t bufSize=1024;
    QBox_Zero(*self);
    QBox_Client_InitEx(self,TestCallAuth,bufSize);

	cJSON* root;
	QBox_Reader body=QBox_FILE_Reader("bazinga");
    err=QBox_Client_CallWithBinary(self, &root, "err", body, 0);
    CU_ASSERT_EQUAL(err.code,1234567);
}

void test_QBox_Client_Call(){
    QBox_Client* self=malloc(sizeof(QBox_Client));
    size_t bufSize=1024;
    QBox_Zero(*self);
    QBox_Client_InitEx(self,TestCallAuth,bufSize);

    QBox_Client* self2=malloc(sizeof(QBox_Client));
    QBox_Zero(*self2);
    QBox_Client_InitNoAuth(self2,bufSize);
	cJSON* root;

    const char* tableName="c_test_table_0";
    const char* key="c_test_key_0";
	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/get/", entryURIEncoded);

	QBox_RS_Create(&client,tableName);
    err=QBox_Client_Call(self, &root, url);
    CU_ASSERT_EQUAL(err.code,612);
	//QBox_RS_Drop(&client,tableName);

    err=QBox_Client_Call(self, &root, "err");
    CU_ASSERT_EQUAL(err.code,1234567);
    err=QBox_Client_Call(self2, &root, url);
    CU_ASSERT_EQUAL(err.code,401);
}

void test_QBox_Client_CallNoRet(){
    QBox_Client* self=malloc(sizeof(QBox_Client));
    size_t bufSize=1024;
    QBox_Zero(*self);
    QBox_Client_InitEx(self,TestCallAuth,bufSize);

    QBox_Client* self2=malloc(sizeof(QBox_Client));
    QBox_Zero(*self2);
    QBox_Client_InitNoAuth(self2,bufSize);

    const char* tableName="c_test_table_0";
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/mkbucket/", tableName);

	QBox_RS_Create(&client,tableName);
    err=QBox_Client_CallNoRet(self, url);
    CU_ASSERT_EQUAL(err.code,200);
	//QBox_RS_Drop(&client,tableName);

    err=QBox_Client_CallNoRet(self, "err");
    CU_ASSERT_EQUAL(err.code,1234567);
    err=QBox_Client_CallNoRet(self2, url);
    CU_ASSERT_EQUAL(err.code,401);
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
    CU_ASSERT_EQUAL(err.code,200);

    err=client.auth.itbl->Auth(client.auth.self,&headers, "err", "test", 4);
    CU_ASSERT_EQUAL(err.code,400);

    client.auth.self=malloc(64);
	QBox_Client_Cleanup(&client);
	free(client.auth.self);
	client.auth.self=NULL;
}
void test_QBox_Oauth2_uptoken(){
    QBox_Client client2;
    QBox_AuthPolicy auth;
    char* uptoken = NULL;

	uptoken = QBox_MakeUpToken(&auth);
    QBox_Client_InitByUpToken(&client2, uptoken, 1024);


    const char* tableName="c_test_table_0";
	char* url = QBox_String_Concat3(QBOX_RS_HOST, "/mkbucket/", tableName);

	//QBox_RS_Create(&client,tableName);
    err=QBox_Client_CallNoRet(&client2, url);
    CU_ASSERT_EQUAL(err.code,401);
	QBox_Client_Cleanup(&client2);
	//QBox_RS_Drop(&client,tableName);
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
/* auth2.c*/
QBOX_TEST(test_QBox_Json_GetString)
QBOX_TEST(test_QBox_Json_GetInt64)
QBOX_TEST(test_QBox_Client_InitEx)
QBOX_TEST(test_QBox_Client_InitNoAuth)
QBOX_TEST(test_QBox_Client_Cleanup)

QBOX_TEST(test_QBox_Client_CallWithBuffer)
QBOX_TEST(test_QBox_Client_CallWithBinary)
QBOX_TEST(test_QBox_Client_Call)
QBOX_TEST(test_QBox_Client_CallNoRet)
/*auth2.c*/
/* auth2_digest.c*/
QBOX_TEST(test_QBox_Oauth2_digest)
/* auth2_digest.c*/
/* auth2_uptoken.c*/
QBOX_TEST(test_QBox_Oauth2_uptoken)
/* auth2_uptoken.c*/
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(oauth2,suite_init_oauth2,suite_clean_oauth2)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsOauth2(void)
{
        QBOX_TEST_REGISTE(oauth2)
}
