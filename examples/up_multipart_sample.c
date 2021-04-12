#include "../qiniu/multipart_upload.h"
#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

void setLocalHost();

void testNotify(Qiniu_UploadPartResp *resp)
{
    Qiniu_Log_Debug("testNotify partNum:%d,remote md5:%s ", resp->partNum, resp->md5);
}
void testNotifyErr(int partNum, Qiniu_Error err)
{
    Qiniu_Log_Error("testNotifyErr partNum:%d,%E", partNum, err);
}

int main(int argc, char **argv)
{
    setLocalHost(); //using localhost for debug
    Qiniu_Global_Init(-1);
    Qiniu_MultipartUpload_Result putRet;
    Qiniu_Client client; //client不支持并发
    Qiniu_RS_PutPolicy putPolicy;
    Qiniu_Multipart_PutExtra putExtra;
    Qiniu_Zero(client); //must initial memory,otherwise will use random ptr;

    char *accessKey = "4_odedBxmrAHiu4Y0Qp0HPG0NANCf6VAsAjWL_k9";
    char *secretKey = "SrRuUVfDX6drVRvpyN8mv8Vcm9XnMZzlbDfvVfMe";
    char *bucket = "sdk";
    char *key = "testkey";
    char *localFile = "./test5m.mp3";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Zero(putPolicy);
    Qiniu_Zero(putExtra);
    putExtra.mimeType = "video/mp4";
    putExtra.enableContentMd5 = 1;
    putExtra.notify = testNotify;
    putExtra.notifyErr = testNotifyErr;

    putPolicy.scope = bucket;
    char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);

    Qiniu_Client_InitMacAuth(&client, 1024, &mac);

    Qiniu_Error error = Qiniu_Multipart_PutFile(&client, uptoken, key, localFile, &putExtra, &putRet);
    if (error.code != 200)
    {
        printf("upload file %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    }
    else
    {
        /*200, 正确返回了, 你可以通过statRet变量查询一些关于这个文件的信息*/
        printf("upload file success dstbucket:%s, dstKey:%s, hash:%s \n\n", bucket, putRet.key, putRet.hash);
    }

    Qiniu_Free(uptoken);
    Qiniu_Client_Cleanup(&client);
}

void setLocalHost()
{
    QINIU_RS_HOST = "http://127.0.0.1:9400";
    QINIU_RSF_HOST = "http://127.0.0.1:10500";
    QINIU_API_HOST = "http://127.0.0.1:12500";
    QINIU_UP_HOST = "http://127.0.0.1:11200";
    QINIU_IOVIP_HOST = "http://127.0.0.1:9200";
}