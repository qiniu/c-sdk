//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_RS_BatchStatRet *statRets;
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
    Qiniu_RS_EntryPath *entries = (Qiniu_RS_EntryPath *) malloc(sizeof(Qiniu_RS_EntryPath) * entryCount);
    for (i = 0; i < entryCount; i++) {
        Qiniu_RS_EntryPath entry;
        entry.bucket = bucket;

        size_t indexLen = snprintf(NULL, 0, "%d", i) + 1;
        char *indexStr = (char *) calloc(sizeof(char), indexLen);
        snprintf(indexStr, indexLen, "%d", i);
        entry.key = Qiniu_String_Concat2(key, indexStr);
        entries[i] = entry;
    }

    statRets = (Qiniu_RS_BatchStatRet *) malloc(sizeof(Qiniu_RS_StatRet) * entryCount);

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_BatchStat(&client, statRets, entries, entryCount);
    if (error.code / 100 != 2) {
        printf("batch stat file error.\n");
        debug_log(&client, error);
    } else {
        /*200, 正确返回了, 你可以通过statRet变量查询一些关于这个文件的信息*/
        printf("batch stat file success.\n\n");

        for (i = 0; i < entryCount; i++) {
            Qiniu_RS_StatRet data = statRets[i].data;
            int code = statRets[i].code;
            if (code == 200) {
                printf("file hash: \t%s\n", data.hash);
                printf("file size: \t%lld\n", data.fsize);
                printf("file put time: \t%lld\n", data.putTime);
                printf("file mime type: \t%s\n", data.mimeType);
                printf("file type: \t%lld\n", data.type);
            } else {
                printf("error: %s\n", statRets[i].error);
            }
        }
    }

    for (i = 0; i < entryCount; i++) {
        Qiniu_RS_EntryPath entry = entries[i];
        Qiniu_Free((void *) entry.key);
    }

    Qiniu_Free(entries);
    Qiniu_Free(statRets);
    Qiniu_Client_Cleanup(&client);
}