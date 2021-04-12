/*
 ============================================================================
 Name        : test_multipart.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../qiniu/multipart_upload.h"
#include "../qiniu/tm.h"

static const char bucket[] = "testbucket";
static const char domain[] = "pw8b601nr.bkt.clouddn.com";

static void clientIoGet(const char *url, Qiniu_Int64 fsize);

static void setLocalHost()
{
    QINIU_RS_HOST = "http://127.0.0.1:9400";
    QINIU_RSF_HOST = "http://127.0.0.1:10500";
    QINIU_API_HOST = "http://127.0.0.1:12500";
    QINIU_UP_HOST = "http://127.0.0.1:11200";
    QINIU_IOVIP_HOST = "http://127.0.0.1:9200";
}

static const char *putFile_multipart(const char *bucket, const char *key, const char *mimeType, const char *filePath, Qiniu_Mac *mac)
{
    Qiniu_Error err;
    Qiniu_Client client;
    Qiniu_Multipart_PutExtra putExtra;
    Qiniu_MultipartUpload_Result putRet;
    Qiniu_RS_PutPolicy putPolicy;
    Qiniu_Zero(client);
    Qiniu_Zero(putPolicy);
    putPolicy.scope = bucket;
    char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, mac);
    // printf("uptoken:%s \n", uptoken);

    Qiniu_Zero(putExtra);
    putExtra.mimeType = mimeType;
    putExtra.enableContentMd5 = 1;
    putExtra.partSize = (4 << 20);
    putExtra.tryTimes = 2;

    Qiniu_Client_InitMacAuth(&client, 1024, mac);

    err = Qiniu_Multipart_PutFile(&client, uptoken, key, filePath, &putExtra, &putRet);
    CU_ASSERT(err.code == 200);

    Qiniu_Log_Info("Qiniu_multipart_putFile: %E", err);

    printf("\n%s\n", Qiniu_Buffer_CStr(&client.respHeader));
    printf("hash: %s , key:%s \n", putRet.hash, putRet.key);

    Qiniu_Client_Cleanup(&client);
    Qiniu_Free(uptoken);
    return putRet.key;
}

void testMultipartUpload_singlePart(void)
{
    setLocalHost();
    Qiniu_Client client;
    Qiniu_Zero(client);

    Qiniu_Error err;
    Qiniu_Mac mac = {QINIU_ACCESS_KEY, QINIU_SECRET_KEY};
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);

    const char *keys[] = {
        "testKey", //normal keyname
        "",        //empty string keyname
        NULL};     //no keyname, determined by server
    for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        const char *inputKey = keys[i];

        //step1: delete  file if exist
        if (inputKey != NULL)
        {
            Qiniu_RS_Delete(&client, bucket, inputKey);
        }
        //step2: upload file
        const char *returnKey = putFile_multipart(bucket, inputKey, "txt", __FILE__, &mac);

        //step3: stat file
        Qiniu_RS_StatRet statResult;
        err = Qiniu_RS_Stat(&client, &statResult, bucket, returnKey);
        CU_ASSERT(err.code == 200);
        CU_ASSERT(strcmp(statResult.mimeType, "txt") == 0);

        //step4: delete file
        err = Qiniu_RS_Delete(&client, bucket, returnKey);
        CU_ASSERT(err.code == 200);
    }

    Qiniu_Client_Cleanup(&client);

    printf("\n testMultipartUpload ok\n\n");
}
