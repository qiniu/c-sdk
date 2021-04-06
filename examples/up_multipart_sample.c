#include "../qiniu/multipart_upload.h"
#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

void Qiniu_Use_Zone_Local();

int main(int argc, char **argv)
{
    Qiniu_Global_Init(-1);
    Qiniu_Multipart_PutRet putRet;
    Qiniu_Client client;
    Qiniu_RS_PutPolicy putPolicy;
    Qiniu_Multipart_PutExtra putExtra;
    putExtra.mimeType = "video/x-mp4";

    char *accessKey = "4_odedBxmrAHiu4Y0Qp0HPG0NANCf6VAsAjWL_k9";
    char *secretKey = "SrRuUVfDX6drVRvpyN8mv8Vcm9XnMZzlbDfvVfMe";
    char *bucket = "sdk";
    char *key = "testkey";
    // char *localFile = "/Users/liangzeng/qbox/sdk/c-sdk/test5m.mp3";
    char *localFile = "/Users/liangzeng/qbox/sdk/c-sdk/testfile";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Zero(putPolicy);
    Qiniu_Zero(putExtra);

    putPolicy.scope = bucket;
    char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);

    Qiniu_Use_Zone_Local();

    Qiniu_Client_InitMacAuth(&client, 1024, &mac);

    Qiniu_Int64 fsize = 0;
    Qiniu_Error error = Qiniu_Multipart_PutWithKey(&client, &putRet, uptoken, key, localFile, &putExtra);
    if (error.code != 200)
    {
        printf("upload file %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    }
    else
    {
        /*200, 正确返回了, 你可以通过statRet变量查询一些关于这个文件的信息*/
        printf("upload file success dstbucket:%s, dstKey:%s \n\n", bucket, key);
        // printf("key:\t%s\n", putRet.key);
        // printf("hash:\t%s\n", putRet.hash);
    }

    Qiniu_Free(uptoken);
    Qiniu_Client_Cleanup(&client);
}

void Qiniu_Use_Zone_Local()
{
    QINIU_RS_HOST = "http://127.0.0.1:9400";
    QINIU_RSF_HOST = "http://127.0.0.1:10500";
    QINIU_API_HOST = "http://127.0.0.1:12500";
    QINIU_UP_HOST = "http://127.0.0.1:11200";
    QINIU_IOVIP_HOST = "http://127.0.0.1:9200";
}