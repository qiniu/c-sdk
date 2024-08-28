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
    Qiniu_Zero(putRet);

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *key = "testKey";
    char *localFile = __FILE__;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Error error;
    Qiniu_Recorder recorder;
    error = Qiniu_FileSystem_Recorder_New("/tmp", &recorder);
    if (error.code != 200)
    {
        printf("create filesystem recorder %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    }

    Qiniu_Zero(putPolicy);
    Qiniu_Zero(putExtra);
    putExtra.mimeType = "video/mp4";
    putExtra.enableContentMd5 = 1;
    putExtra.notify = testNotify;
    putExtra.notifyErr = testNotifyErr;
    putExtra.metaCount = 2;
    putExtra.metaList = malloc(putExtra.metaCount * sizeof(char *[2]));
    putExtra.metaList[0][0] = "metakey1";
    putExtra.metaList[0][1] = "metaval1";
    putExtra.metaList[1][0] = "metakey2";
    putExtra.metaList[1][1] = "metaval2";
    putExtra.recorder = &recorder;

    putPolicy.scope = bucket;
    char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);

    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);
    Qiniu_RS_Delete(&client, bucket, key); //to avoid "file exist" err

    error = Qiniu_Multipart_PutFile(&client, uptoken, key, localFile, &putExtra, &putRet);
    if (error.code != 200)
    {
        printf("upload file %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    }
    else
    {
        printf("upload file success dstbucket:%s, dstKey:%s, hash:%s \n\n", bucket, putRet.key, putRet.hash);
    }
    recorder.free(&recorder);
    Qiniu_Free(putExtra.metaList);
    Qiniu_Free(uptoken);
    Qiniu_Free(putRet.key);
    Qiniu_Free(putRet.hash);
    Qiniu_Client_Cleanup(&client);
}

void setLocalHost()
{
#ifdef LOCAL_DEBUG_MODE //dedicated for qiniu maintainer
    QINIU_RS_HOST = "http://127.0.0.1:9400";
    QINIU_UP_HOST = "http://127.0.0.1:11200";
#else
    Qiniu_Use_Zone_Huadong(false);
#endif
}
