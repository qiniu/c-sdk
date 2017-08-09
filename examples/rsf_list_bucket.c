//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rsf.h"
#include <stdlib.h>
#include <string.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_RSF_ListRet listRet;
    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *prefix = "";
    char *delimiter = "/";
    char *marker = "";
    char *nextMarker = marker;
    int limit = 3;
    int i;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    char **commonPrefixes = NULL;
    Qiniu_RSF_ListItem *items = NULL;

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);

    do {
        Qiniu_Error error = Qiniu_RSF_ListFiles(&client, &listRet, bucket, prefix, delimiter, nextMarker, limit);
        if (strcmp(nextMarker, "") != 0) {
            Qiniu_Free(nextMarker);
        }
        if (error.code != 200) {
            printf("list files of bucket %s error.\n", bucket);
            debug_log(&client, error);
        } else {
            /*200, 正确返回了, 你可以通过listRet变量查询文件列表信息*/
            printf("list files of bucket %s success.\n\n", bucket);

            //check next marker
            if (listRet.marker) {
                size_t markerLen = strlen(listRet.marker) + 1;
                nextMarker = (char *) malloc(sizeof(char) * markerLen);
                memset(nextMarker, 0, markerLen);
                snprintf(nextMarker, markerLen, "%s", listRet.marker);
            } else {
                nextMarker = 0;
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
        }
    } while (nextMarker != 0);

    Qiniu_Client_Cleanup(&client);
}