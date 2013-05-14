/*
 ============================================================================
 Name        : suites_http.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <curl/curl.h>
 #include <openssl/hmac.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qiniu/conf.h"
#include "../qiniu/http.h"
#include "../qiniu/resumable_io.h"
#include "test.h"

static void test_Qiniu_Json_GetString(){
    Qiniu_Json json;
    json.next=NULL;
    json.type=cJSON_String;
    json.string=malloc(sizeof(char)*56);
    strcpy(json.string,"test");
    json.valuestring=malloc(sizeof(char)*56);
    strcpy(json.valuestring,"test_valuestring");

    Qiniu_Json jsonArray;
    jsonArray.child=&json;
    char* str=NULL;
    //test brach:sub != NULL  sub->type == cJSON_String
    str=Qiniu_Json_GetString(&jsonArray,"test","not_find");
    CU_ASSERT_STRING_EQUAL(str,"test_valuestring");
    //test brach:sub == NULL
    str=Qiniu_Json_GetString(&jsonArray,"test_not_find","not_find");
    CU_ASSERT_STRING_EQUAL(str,"not_find");
    //test brach:sub != NULL  sub->type != cJSON_String
    json.type=cJSON_Number;
    str=Qiniu_Json_GetString(&jsonArray,"test","not_find");
    CU_ASSERT_STRING_EQUAL(str,"not_find");
    //test brach:self == NULL
    str=Qiniu_Json_GetString(NULL,"test","not_find");
    CU_ASSERT_STRING_EQUAL(str,"not_find");
}

static void test_Qiniu_Json_GetInt64(){
    Qiniu_Json json;
    json.next=NULL;
    json.type=cJSON_Number;
    json.string=malloc(sizeof(char)*56);
    strcpy(json.string,"test");
    json.valuestring=malloc(sizeof(char)*56);
    strcpy(json.valuestring,"test_valuestring");
    json.valuedouble=333;

    Qiniu_Json jsonArray;
    jsonArray.child=&json;
    int rlt;
    //test branch:sub != NULL sub->type == cJSON_Number
    rlt=Qiniu_Json_GetInt64(&jsonArray,"test",999);
    CU_ASSERT_EQUAL(rlt,333);
    //test branch:sub == NULL
    rlt=Qiniu_Json_GetInt64(&jsonArray,"test_not_find",999);
    CU_ASSERT_EQUAL(rlt,999);
    //test branch:sub != NULL sub->type != cJSON_Number
    json.type=cJSON_String;
    rlt=Qiniu_Json_GetInt64(&jsonArray,"test",999);
    CU_ASSERT_EQUAL(rlt,999);
    //test brach:self == NULL
    rlt=Qiniu_Json_GetInt64(NULL,"test",999);
    CU_ASSERT_EQUAL(rlt,999);
}

static Qiniu_Error AuthForTest_Auth(void* self, Qiniu_Header** header, const char* url, const char* addition, size_t addlen)
{
    Qiniu_Error error;
    error.code=200;
    return error;
}
static void AuthForTest_Release(void* self)
{
}
static Qiniu_Auth_Itbl Auth_Itbl_ForTest = {
	AuthForTest_Auth,
	AuthForTest_Release
};
static Qiniu_Auth AuthForTest = {
	"test",
	&Auth_Itbl_ForTest
};

static void test_Qiniu_Client_InitEx(){
    Qiniu_Client client;
    size_t bufSize=1024;
    Qiniu_Zero(client);

    Qiniu_Client_InitEx(&client,AuthForTest,bufSize);
    CU_ASSERT_STRING_EQUAL(client.curl,"");
    CU_ASSERT_EQUAL(client.root,NULL);
    CU_ASSERT_STRING_EQUAL(client.auth.self,"test");
    CU_ASSERT_EQUAL(client.b.bufEnd-client.b.buf,bufSize);
    CU_ASSERT_EQUAL(client.respHeader.bufEnd-client.respHeader.buf,bufSize);

    Qiniu_Client_Cleanup(&client);
}

static void test_Qiniu_Client_InitNoAuth(){
    Qiniu_Client client;
    size_t bufSize=1024;
    Qiniu_Zero(client);

    Qiniu_Client_InitNoAuth(&client,bufSize);

    CU_ASSERT_STRING_EQUAL(client.curl,"");
    CU_ASSERT_EQUAL(client.root,NULL);
    CU_ASSERT_EQUAL(client.auth.self,NULL);
    CU_ASSERT_EQUAL(client.auth.itbl,NULL);
    CU_ASSERT_EQUAL(client.b.bufEnd-client.b.buf,bufSize);
    CU_ASSERT_EQUAL(client.respHeader.bufEnd-client.respHeader.buf,bufSize);

    Qiniu_Client_Cleanup(&client);
}


static void test_Qiniu_Client_Cleanup(){
    Qiniu_Client client;
    size_t bufSize=1024;
    Qiniu_Zero(client);

    Qiniu_Client_InitEx(&client,AuthForTest,bufSize);
    client.root=malloc(sizeof(Qiniu_Json));
    client.root->next=NULL;
    client.root->type=cJSON_Number;
    client.root->string=NULL;
    client.root->valuestring=NULL;
    client.root->valuedouble=333;
    client.root->child=NULL;

    Qiniu_Client_Cleanup(&client);
    CU_ASSERT_EQUAL(client.auth.itbl,NULL);
    CU_ASSERT_EQUAL(client.curl,NULL);
    CU_ASSERT_EQUAL(client.root,NULL);
    CU_ASSERT_EQUAL(client.b.buf,NULL);
    CU_ASSERT_EQUAL(client.respHeader.buf,NULL);

    //test brach:self->auth.itbl =self->curl =self->root= NULL
    Qiniu_Client_Cleanup(&client);
    CU_ASSERT_EQUAL(client.auth.itbl,NULL);
    CU_ASSERT_EQUAL(client.curl,NULL);
    CU_ASSERT_EQUAL(client.root,NULL);
    CU_ASSERT_EQUAL(client.b.buf,NULL);
    CU_ASSERT_EQUAL(client.respHeader.buf,NULL);
}


static Qiniu_Error TestCall_Auth(void* self, Qiniu_Header** header, const char* url, const char* addition, size_t addlen)
{
    Qiniu_Error error;
    if(strcmp(url,"err")==0){
        error.code=1234567;
        return error;
    }

	Qiniu_Error err;
	char* auth;
	char* enc_digest;
	char digest[EVP_MAX_MD_SIZE + 1];
	unsigned int dgtlen = sizeof(digest);
	HMAC_CTX ctx;
	Qiniu_Mac mac;
	char const* path = strstr(url, "://");
	if (path != NULL) {
		path = strchr(path + 3, '/');
	}
    printf("\n%s\n",path);
	if (path == NULL) {
		err.code = 400;
		err.message = "invalid url";
		return err;
	}

    mac.accessKey = QINIU_ACCESS_KEY;
    mac.secretKey = QINIU_SECRET_KEY;

	HMAC_CTX_init(&ctx);
	HMAC_Init_ex(&ctx, mac.secretKey, strlen(mac.secretKey), EVP_sha1(), NULL);
	HMAC_Update(&ctx, path, strlen(path));
	HMAC_Update(&ctx, "\n", 1);
	if (addlen > 0) {
		HMAC_Update(&ctx, addition, addlen);
	}

	HMAC_Final(&ctx, digest, &dgtlen);
	HMAC_CTX_cleanup(&ctx);

	enc_digest = Qiniu_Memory_Encode(digest, dgtlen);

	auth = Qiniu_String_Concat("Authorization: QBox ", mac.accessKey, ":", enc_digest, NULL);
	free(enc_digest);

	*header = curl_slist_append(*header, auth);
	free(auth);
	return Qiniu_OK;
}
static void TestCall_Release(void* self)
{
}
static Qiniu_Auth_Itbl TestCallAuth_Itbl = {
	TestCall_Auth,
	TestCall_Release
};
static Qiniu_Auth TestCallAuth = {
	"TestCall",
	&TestCallAuth_Itbl
};
static void test_Qiniu_Client_CallWithBuffer(){
    size_t bufSize=1024;
    Qiniu_Client client;
    Qiniu_Zero(client);
    Qiniu_Client_InitEx(&client,TestCallAuth,bufSize);


    Qiniu_Client client2;
    Qiniu_Zero(client2);
    Qiniu_Client_InitNoAuth(&client2,bufSize);

    Qiniu_Int64 fsize=0;
    const char* key="test_Qiniu_Client_CallWithBuffer";
    const char* bucket="Bucket";

	size_t i, blkCount = 0;
	Qiniu_Json* root;
	Qiniu_Error err;
	Qiniu_Rio_BlkputRet* prog;
	Qiniu_Buffer url, body;

	char* entry = Qiniu_String_Concat3(bucket, ":", key);

	Qiniu_Buffer_Init(&url, 1024);
	Qiniu_Buffer_AppendFormat(&url, "%s/rs-mkfile/%S/fsize/%D", QINIU_UP_HOST, entry, fsize);
	free(entry);


	Qiniu_Buffer_Init(&body, 176 * blkCount);
	if (blkCount > 0) {
		body.curr--;
	}
    printf("\n%s\n",Qiniu_Buffer_CStr(&url));
	err = Qiniu_Client_CallWithBuffer(
		&client, &root, Qiniu_Buffer_CStr(&url), body.buf, body.curr - body.buf, "text/plain");
    printf("\n%d %s\n",err.code,err.message);
    CU_ASSERT_EQUAL(err.code,200);
	//QBox_RS_Drop(&client,"c_test_table_0");
/*
    //test self->auth.itbl != NULL err.code != 200
    chkBuf = malloc(bodyLength);
    err=QBox_Client_CallWithBuffer(self, root, "err", NULL, 0);
    CU_ASSERT_EQUAL(err.code,1234567);

    //test self->auth.itbl== NULL err
    chkBuf = malloc(bodyLength);
    err=QBox_Client_CallWithBuffer(self2, &root, url, chkBuf, bodyLength);
    CU_ASSERT_EQUAL(err.code,401);
    */
}

static void test_Mac(){
    Qiniu_MacAuth_Init();
    Qiniu_MacAuth_Cleanup();
    Qiniu_MacAuth_Init();
    Qiniu_MacAuth_Cleanup();
	Qiniu_Global_Cleanup();
	Qiniu_Servend_Init(-1);
	Qiniu_Servend_Cleanup();
	Qiniu_Global_Init(-1);
	Qiniu_Mac mac;
	Qiniu_Mac *pointer_mac;
	Qiniu_Auth auth;
	auth=Qiniu_MacAuth(NULL);
	pointer_mac=auth.self;
	CU_ASSERT_EQUAL(pointer_mac,NULL);
	mac.accessKey=malloc(sizeof(32));
	mac.secretKey=malloc(sizeof(32));
	strcpy(mac.accessKey,"test");
	strcpy(mac.secretKey,"test");

	auth=Qiniu_MacAuth(&mac);
	pointer_mac=auth.self;
	CU_ASSERT_STRING_EQUAL(pointer_mac->accessKey,mac.accessKey);
	CU_ASSERT_STRING_EQUAL(pointer_mac->secretKey,mac.secretKey);


}



/**//*---- test suites ------------------*/
static int suite_init(void)
{

	QINIU_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QINIU_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	Qiniu_Global_Init(-1);

	return 0;
}

static int suite_clean(void)
{
	Qiniu_Global_Cleanup();
    return 0;
}

QINIU_TESTS_BEGIN(testcases_http)
QINIU_TEST(test_Qiniu_Json_GetString)
QINIU_TEST(test_Qiniu_Json_GetInt64)
QINIU_TEST(test_Qiniu_Client_InitEx)
QINIU_TEST(test_Qiniu_Client_InitNoAuth)
QINIU_TEST(test_Qiniu_Client_Cleanup)
//QINIU_TEST(test_Qiniu_Client_CallWithBuffer)
QINIU_TEST(test_Mac)
QINIU_TESTS_END()

QINIU_SUITES_BEGIN()
QINIU_SUITE_EX(testcases_http,suite_init,suite_clean)
QINIU_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsHttp(void)
{
        QINIU_TEST_REGISTE(http)
}

