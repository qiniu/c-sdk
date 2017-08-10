//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_Global_Init(-1);

    Qiniu_RS_BatchItemRet *itemRets;
    Qiniu_Client client;
    int i;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *key = "qiniu.png";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_ItemCount entryCount = 10;
    Qiniu_RS_EntryChangeMime *entries = (Qiniu_RS_EntryChangeMime *) malloc(
            sizeof(Qiniu_RS_EntryChangeMime) * entryCount);
    for (i = 0; i < entryCount; i++) {
        Qiniu_RS_EntryChangeMime entry;
        entry.bucket = bucket;

        size_t indexLen = snprintf(NULL, 0, "%d", i) + 1;
        char *indexStr = (char *) calloc(sizeof(char), indexLen);
        snprintf(indexStr, indexLen, "%d", i);
        entry.key = Qiniu_String_Concat2(key, indexStr);
        Qiniu_Free(indexStr);
        entry.mime = "image/x-png";
        entries[i] = entry;
    }

    itemRets = (Qiniu_RS_BatchItemRet *) malloc(sizeof(Qiniu_RS_BatchItemRet) * entryCount);

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_BatchChangeMime(&client, itemRets, entries, entryCount);
    if (error.code / 100 != 2) {
        printf("batch change file mime error.\n");
        debug_log(&client, error);
    } else {
        /*200, 正确返回了, 你可以通过itemRets变量查询一些关于这个文件的信息*/
        printf("batch change file mime success.\n\n");

        for (i = 0; i < entryCount; i++) {
            int code = itemRets[i].code;
            if (code == 200) {
                printf("success: %d\n", code);
            } else {
                printf("code: %d, error: %s\n", code, itemRets[i].error);
            }
        }
    }

    for (i = 0; i < entryCount; i++) {
        Qiniu_RS_EntryChangeMime entry = entries[i];
        Qiniu_Free((void *) entry.key);
    }

    Qiniu_Free(entries);
    Qiniu_Free(itemRets);
    Qiniu_Client_Cleanup(&client);
}