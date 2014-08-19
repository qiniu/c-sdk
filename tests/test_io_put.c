/*
 ============================================================================
 Name        : test_io_put.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"
#include "../qiniu/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

static const char bucket[] = "csdk";
static const char key[] = "key";
static const char domain[] = "csdk.qiniudn.com";

static void clientIoPutFile(const char* uptoken)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Io_PutRet putRet;

	Qiniu_Zero(putRet);
	Qiniu_Client_InitNoAuth(&client, 1024);

	err = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, __FILE__, NULL);
	CU_ASSERT(err.code == 200);

	printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash?putRet.hash:"");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoPutBuffer(const char* uptoken)
{
	const char text[] = "Hello, world!";

	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Io_PutRet putRet;

	Qiniu_Zero(putRet);
	Qiniu_Client_InitNoAuth(&client, 1024);

	err = Qiniu_Io_PutBuffer(&client, &putRet, uptoken, key, text, sizeof(text)-1, NULL);

	printf("\n%s", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash?putRet.hash:"");

	CU_ASSERT(err.code == 200);
	CU_ASSERT_STRING_EQUAL(putRet.hash, "FpQ6cC0G80WZruH42o759ylgMdaZ");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoGet(const char* url)
{
	const char text[] = "Hello, world!";

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

void testIoPut(void)
{
	Qiniu_Client client;
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_RS_GetPolicy getPolicy;
	char* uptoken;
	char* dnBaseUrl;
	char* dnRequest;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);

	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, NULL);

	Qiniu_RS_Delete(&client, bucket, key);
	clientIoPutFile(uptoken);

	Qiniu_RS_Delete(&client, bucket, key);
	clientIoPutBuffer(uptoken);

	Qiniu_Free(uptoken);

	Qiniu_Zero(getPolicy);
	dnBaseUrl = Qiniu_RS_MakeBaseUrl(domain, key);
	dnRequest = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, dnBaseUrl, NULL);

	clientIoGet(dnRequest);

	Qiniu_Free(dnRequest);
	Qiniu_Free(dnBaseUrl);

	Qiniu_Client_Cleanup(&client);
}

