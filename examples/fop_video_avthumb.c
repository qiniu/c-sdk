//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/fop.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
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

    //init
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

    Qiniu_FOP_PfopParams params = {
        .bucket = bucket,
        .key = key,
        .pipeline = pipeline,
        .notifyURL = notifyURL,
        .fops = fops,
        .fopCount = 2,
        .force = force,
    };
    Qiniu_Error error = Qiniu_FOP_Pfop_v2(&client, &pfopRet, &params);
    if (error.code != 200) {
        printf("video file pfop %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    } else {
        /*200, 正确返回了, 你可以通过pfopRet变量查询任务ID*/
        printf("video file pfop %s:%s success, persistentId: %s .\n\n", bucket, key, pfopRet.persistentId);
    }

    Qiniu_FOP_PrefopRet prefopRet;
    Qiniu_FOP_PrefopItemRet prefopItemRet[2];
    Qiniu_ItemCount itemsCount;
    error = Qiniu_FOP_Prefop(&client, &prefopRet, (Qiniu_FOP_PrefopItemRet *)&prefopItemRet, &itemsCount, pfopRet.persistentId, 2);
    if (error.code != 200)
    {
        debug_log(&client, error);
    }
    else
    {
        printf("ID: %s\n", prefopRet.id);
        printf("Code: %d\n", prefopRet.code);
        printf("Desc: %s\n", prefopRet.desc);
        printf("InputBucket: %s\n", prefopRet.inputBucket);
        printf("InputKey: %s\n", prefopRet.inputKey);
        printf("TaskFrom: %s\n", prefopRet.taskFrom);
        printf("Type: %d\n", prefopRet.type);
        printf("CreationDate: %d-%d-%d %d:%d:%d +%d\n", prefopRet.creationDate.date.year, prefopRet.creationDate.date.month,
               prefopRet.creationDate.date.day, prefopRet.creationDate.time.hour,
               prefopRet.creationDate.time.minute, prefopRet.creationDate.time.second,
               prefopRet.creationDate.time.offset);

        for (Qiniu_ItemCount i = 0; i < itemsCount; i++)
        {
            printf("\tIndex: %d\n", i);
            printf("\tCmd: %s\n", prefopItemRet[i].cmd);
            printf("\tCode: %d\n", prefopItemRet[i].code);
            printf("\tDesc: %s\n", prefopItemRet[i].desc);
            printf("\tError: %s\n", prefopItemRet[i].error);
            printf("\tHash: %s\n", prefopItemRet[i].hash);
            printf("\tKey: %s\n", prefopItemRet[i].key);
            printf("\tReturnOld: %d\n", prefopItemRet[i].returnOld);
        }
    }

    Qiniu_Free(avthumbMp4Fop);
    Qiniu_Free(vframeJpgFop);
    Qiniu_Client_Cleanup(&client);
}
