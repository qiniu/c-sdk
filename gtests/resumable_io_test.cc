#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "test.h"
#include "qiniu/rs.h"
#include "qiniu/resumable_io.h"

static void clientIoPutFile(const char *uptoken, const char *key)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;

	Qiniu_Client_InitNoAuth(&client, 1024);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	Qiniu_Zero(extra);

	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, __FILE__, &extra);
	EXPECT_EQ(err.code, Qiniu_OK.code);

	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Log_Debug("hash: %s", putRet.hash);

	Qiniu_Client_Cleanup(&client);
}

static int notify(void *self, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet *ret)
{
	Qiniu_Log_Info("nodify: %d, off: %d", blkIdx, ret->offset);
	return QINIU_RIO_NOTIFY_OK;
}

static int notifyErr(void *self, int blkIdx, int blkSize, Qiniu_Error err)
{
	Qiniu_Log_Warn("nodify: %d, err: %E", blkIdx, err);
	return QINIU_RIO_NOTIFY_OK;
}

static const Qiniu_Int64 testFsize = 4 * 1024 + 2;

static void clientIoPutBuffer(const char *uptoken, const char *key)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;
	Qiniu_Seq seq;
	Qiniu_ReaderAt in;
	Qiniu_Int64 fsize = testFsize;

	Qiniu_Client_InitNoAuth(&client, 1024);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	Qiniu_Zero(extra);

	extra.notify = notify;
	extra.notifyErr = notifyErr;
	extra.chunkSize = 1024;

	in = Qiniu_SeqReaderAt(&seq, (size_t)fsize, 10, '0', 0);

	err = Qiniu_Rio_Put(&client, &putRet, uptoken, key, in, fsize, &extra);

	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Log_Debug("hash: %s", putRet.hash);

	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_STREQ(putRet.hash, "FoErrxvY99fW7npWmVii0RncWKme");

	Qiniu_Client_Cleanup(&client);
}

static void clientIoGet(const char *url)
{
	Qiniu_Eq eq;
	Qiniu_Seq seq;
	Qiniu_Int64 fsize = testFsize;
	Qiniu_Reader in = Qiniu_SeqReader(&seq, (size_t)fsize, 10, '0', 0);
	Qiniu_Writer w = Qiniu_EqWriter(&eq, in);

	long code, httpCode;
	CURL *curl = curl_easy_init();
	Qiniu_Buffer respHeader;
	Qiniu_Buffer_Init(&respHeader, 1024);

	Qiniu_Log_Debug("url: %s", url);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, w.Write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, w.self);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Qiniu_Buffer_Fwrite);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &respHeader);

	code = curl_easy_perform(curl);

	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&respHeader));
	Qiniu_Log_Debug("curl.code: %d, eq.result: %d", (int)code, eq.result);
	EXPECT_EQ(code, 0);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	EXPECT_EQ(httpCode, Qiniu_OK.code);

	EXPECT_TRUE(Qiniu_Is(&eq));

	curl_easy_cleanup(curl);
}

TEST(IntegrationTest, TestResumableIoPut)
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
