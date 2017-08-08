//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rsf.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_RSF_ListRet listRet;
    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = "csdk";
    char *prefix = "";
    char *delimiter = "/";
    char *marker = "";
    int limit = 3;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_RSF_CommonPrefix *commonPrefix = NULL;
    Qiniu_RSF_CommonPrefix *nextPrefix = NULL;

    Qiniu_RSF_ListItem *item = NULL;
    Qiniu_RSF_ListItem *nextItem = NULL;

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RSF_ListFiles(&client, &listRet, bucket, prefix, delimiter, marker, limit);
    if (error.code != 200) {
        printf("list files of bucket %s error.\n", bucket);
        debug_log(&client, error);
    } else {
        /*200, 正确返回了, 你可以通过listRet变量查询文件列表信息*/
        printf("list files of bucket %s success.\n\n", bucket);
        //marker
        printf("next marker: %s\n", listRet.marker);

        //common prefixes
        commonPrefix = listRet.commonPrefix;
        while (commonPrefix) {
            printf("commonPrefix: %s\n", commonPrefix->value);
            commonPrefix = commonPrefix->next;
        }

        //items
        item = listRet.item;
        while (item) {
            printf("key: %s, hash: %s, fsize: %lld, mime: %s, putTime: %lld, endUser: %s, type: %lld\n",
                   item->key, item->hash, item->fsize, item->mimeType, item->putTime, item->endUser, item->type);
            item = item->next;
        }

        //free common prefixes
        commonPrefix = listRet.commonPrefix;
        nextPrefix = commonPrefix;
        while (nextPrefix) {
            commonPrefix = commonPrefix->next;
            Qiniu_Free((void *) nextPrefix->value);
            Qiniu_Free(nextPrefix);
            nextPrefix = commonPrefix;
        }

        //free items
        item = listRet.item;
        nextItem = item;
        while (nextItem) {
            item = item->next;
            Qiniu_Free((void *) nextItem->key);
            Qiniu_Free((void *) nextItem->hash);
            Qiniu_Free((void *) nextItem->mimeType);
            if (nextItem->endUser) {
                Qiniu_Free((void *) nextItem->endUser);
            }
            Qiniu_Free(nextItem);
            nextItem = item;
        }
    }

    Qiniu_Client_Cleanup(&client);
}