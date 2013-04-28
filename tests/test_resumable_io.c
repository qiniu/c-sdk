/*
 ============================================================================
 Name        : test_io_put.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"
#include "../qiniu/resumable_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

static const char bucket[] = "a";
static const char key[] = "key2";
static const char domain[] = "aatest.qiniudn.com";

static void clientIoPutFile(const char* uptoken)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Rio_PutRet putRet;

	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Zero(extra);
	extra.bucket = bucket;

	err = Qiniu_Rio_PutFile(&client, &putRet, uptoken, key, __FILE__, &extra);
	CU_ASSERT(err.code == 200);

	Qiniu_Log_Infof("Qiniu_Rio_PutFile: %E", err);

	printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));
	printf("hash: %s\n", putRet.hash);

	Qiniu_Client_Cleanup(&client);
}

void testResumableIoPut()
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

	free(uptoken);

	Qiniu_Client_Cleanup(&client);
}

