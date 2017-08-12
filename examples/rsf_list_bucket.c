//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rsf.h"
#include <stdlib.h>
#include <string.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_Global_Init(-1);

    Qiniu_RSF_ListRet listRet;
    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *prefix = "";
    char *delimiter = "/";
    char *marker = "";
    char *nextMarker = marker;
    Qiniu_Bool isNewMarker = Qiniu_False;
    int limit = 10;
    int i;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    char **commonPrefixes = NULL;
    Qiniu_RSF_ListItem *items = NULL;

    do {
        //init
        Qiniu_Client_InitMacAuth(&client, 1024, &mac);
        Qiniu_Error error = Qiniu_RSF_ListFiles(&client, &listRet, bucket, prefix, delimiter, nextMarker, limit);

        if (error.code != 200) {
            printf("list files of bucket %s error.\n", bucket);
            debug_log(&client, error);
            nextMarker = "";
        }
        else {
            /*200, 正确返回了, 你可以通过listRet变量查询文件列表信息*/
            printf("list files of bucket %s success.\n\n", bucket);

            //check next marker
            if (isNewMarker == Qiniu_True) {
                free(nextMarker);
            }

            if (!str_empty(listRet.marker)) {
                nextMarker = strdup(listRet.marker);
                isNewMarker = Qiniu_True;
            }
            else {
                nextMarker = NULL;
            }
            printf("next marker: %s\n", nextMarker);

            //common prefixes
            commonPrefixes = listRet.commonPrefixes;
            for (i = 0; i < listRet.commonPrefixesCount; i++) {
                printf("commonPrefix: %s\n", *commonPrefixes);
                ++commonPrefixes;
            }

            //items
            items = listRet.items;
            for (i = 0; i < listRet.itemsCount; i++) {
                Qiniu_RSF_ListItem item = listRet.items[i];
                printf("key: %s, hash: %s, fsize: %lld, mime: %s, putTime: %lld, endUser: %s, type: %lld\n",
                       item.key, item.hash, item.fsize, item.mimeType, item.putTime, item.endUser, item.type);
            }

            //free
            if (listRet.commonPrefixes != NULL) {
                Qiniu_Free(listRet.commonPrefixes);
            }
            if (listRet.items != NULL) {
                Qiniu_Free(listRet.items);
            }

            Qiniu_Zero(listRet);
        }

        Qiniu_Client_Cleanup(&client);
    } while (!str_empty(nextMarker));
}