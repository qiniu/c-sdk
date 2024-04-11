//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/fop.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv)
{
    Qiniu_Global_Init(-1);

    Qiniu_FOP_PfopRet pfopRet;
    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *key = "qiniu.mp4";
    char *pipeline = "sdktest";
    char *notifyURL = NULL;
    int force = 0;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    // init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);

    char *saveMp4Entry = Qiniu_String_Concat3(bucket, ":", "avthumb_test_target.mp4");
    char *saveMp4EntryEncoded = Qiniu_String_Encode(saveMp4Entry);
    Qiniu_Free(saveMp4Entry);

    char *saveJpgEntry = Qiniu_String_Concat3(bucket, ":", "vframe_test_target.jpg");
    char *saveJpgEntryEncoded = Qiniu_String_Encode(saveJpgEntry);
    Qiniu_Free(saveJpgEntry);

    char *avthumbMp4Fop = Qiniu_String_Concat2("avthumb/mp4|saveas/", saveMp4EntryEncoded);
    char *vframeJpgFop = Qiniu_String_Concat2("vframe/jpg/offset/1|saveas/", saveJpgEntryEncoded);

    Qiniu_Free(saveMp4EntryEncoded);
    Qiniu_Free(saveJpgEntryEncoded);

    char *fops[] = {avthumbMp4Fop, vframeJpgFop};

    Qiniu_Error error = Qiniu_FOP_Pfop(&client, &pfopRet, bucket, key, fops, 2, pipeline, notifyURL, force);
    if (error.code != Qiniu_OK.code)
    {
        printf("video file pfop %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    }
    else
    {
        /*Qiniu_OK.code, 正确返回了, 你可以通过pfopRet变量查询任务ID*/
        printf("video file pfop %s:%s success, persistentId: %s .\n\n", bucket, key, pfopRet.persistentId);
    }

    Qiniu_Free(avthumbMp4Fop);
    Qiniu_Free(vframeJpgFop);
    Qiniu_Client_Cleanup(&client);
}
