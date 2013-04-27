/*
 ============================================================================
 Name        : test_io_put.c
 Author      : Qiniu.com
 Version     : 1.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "../qiniu/rs.h"
#include "../qiniu/io.h"
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

static const char bucket[] = "a";
static const char key[] = "key";
static const char domain[] = "aatest.qiniudn.com";

static void clientIoPutFile(const char* uptoken)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Io_PutExtra extra;
	Qiniu_Io_PutRet putRet;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

	err = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, __FILE__, &extra);
	CU_ASSERT(err.code == 200);

	printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

	Qiniu_Client_Cleanup(&client);
}

static void clientIoPutBuffer(const char* uptoken)
{
	const char text[] = "Hello, world!";

	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Io_PutExtra extra;
	Qiniu_Io_PutRet putRet;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

	err = Qiniu_Io_PutBuffer(&client, &putRet, uptoken, key, text, sizeof(text)-1, &extra);

	printf("\n%s", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

	CU_ASSERT(err.code == 200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FpQ6cC0G80WZruH42o759ylgMdaZ");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoGet(const char* dntoken)
{
	const char text[] = "Hello, world!";
	char* url = Qiniu_String_Concat("http://", domain, "/", key, "?token=", dntoken, NULL);

	long code, httpCode;
	CURL* curl = curl_easy_init();
	Qiniu_Buffer resp;
	Qiniu_Buffer respHeader;

	Qiniu_Buffer_Init(&resp, 1024);
	Qiniu_Buffer_Init(&respHeader, 1024);

	printf("url: %s\n", url);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Qiniu_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Qiniu_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &respHeader);

	code = curl_easy_perform(curl);

	printf("\n%s", Qiniu_Buffer_CStr(&respHeader));
	CU_ASSERT(code == 0);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	CU_ASSERT(httpCode == 200);

	CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(&resp), text);

	curl_easy_cleanup(curl);
}

void testIoPut()
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_RS_GetPolicy getPolicy;
	char* uptoken;
	char* dntoken;

	Qiniu_Client_Init(&client, 1024);

	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy);

	Qiniu_RS_Delete(&client, bucket, key);
	clientIoPutFile(uptoken);

	Qiniu_RS_Delete(&client, bucket, key);
	clientIoPutBuffer(uptoken);

	free(uptoken);

	Qiniu_Zero(getPolicy);
	getPolicy.scope = "*/*";
	dntoken = Qiniu_RS_GetPolicy_Token(&getPolicy);

	clientIoGet(dntoken);

	free(dntoken);

	Qiniu_Client_Cleanup(&client);
}
