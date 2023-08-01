#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "test.h"
#include "qiniu/io.h"
#include "qiniu/rs.h"

static void clientIoPutFile(const char *uptoken, const char *key)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Io_PutRet putRet;

	Qiniu_Zero(putRet);
	Qiniu_Client_InitNoAuth(&client, 1024);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	err = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, __FILE__, NULL);
	EXPECT_EQ(err.code, 200);

	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Log_Debug("hash: %s", putRet.hash ? putRet.hash : "");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoPutBuffer(const char *uptoken, const char *key)
{
	const char text[] = "Hello, world!";

	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Io_PutRet putRet;

	Qiniu_Zero(putRet);
	Qiniu_Client_InitNoAuth(&client, 1024);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	err = Qiniu_Io_PutBuffer(&client, &putRet, uptoken, key, text, sizeof(text) - 1, NULL);

	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Log_Debug("hash: %s", putRet.hash ? putRet.hash : "");

	EXPECT_EQ(err.code, 200);
	EXPECT_STREQ(putRet.hash, "FpQ6cC0G80WZruH42o759ylgMdaZ");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoGet(const char *url)
{
	const char text[] = "Hello, world!";

	long code, httpCode;
	CURL *curl = curl_easy_init();
	Qiniu_Buffer resp;
	Qiniu_Buffer respHeader;

	Qiniu_Buffer_Init(&resp, 1024);
	Qiniu_Buffer_Init(&respHeader, 1024);

	Qiniu_Log_Debug("url: %s", url);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Qiniu_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Qiniu_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &respHeader);

	code = curl_easy_perform(curl);

	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&respHeader));
	EXPECT_EQ(code, 0);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	EXPECT_EQ(httpCode, 200);

	EXPECT_STREQ(Qiniu_Buffer_CStr(&resp), text);

	curl_easy_cleanup(curl);
}

TEST(IntegrationTest, TestIoPut)
{
	Qiniu_Client client;
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_RS_GetPolicy getPolicy;
	char *uptoken;
	char *dnBaseUrl;
	char *dnRequest;
	char key[100];
	Qiniu_snprintf(key, 100, "key_%d", rand());

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	Qiniu_Zero(putPolicy);
	putPolicy.scope = Test_bucket;
	uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, NULL);

	Qiniu_RS_Delete(&client, Test_bucket, key);
	clientIoPutFile(uptoken, key);

	Qiniu_RS_Delete(&client, Test_bucket, key);
	clientIoPutBuffer(uptoken, key);

	Qiniu_Free(uptoken);

	Qiniu_Zero(getPolicy);
	dnBaseUrl = Qiniu_RS_MakeBaseUrl(Test_Domain, key);
	dnRequest = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, dnBaseUrl, NULL);

	clientIoGet(dnRequest);

	Qiniu_Free(dnRequest);
	Qiniu_Free(dnBaseUrl);

	Qiniu_Client_Cleanup(&client);
}
