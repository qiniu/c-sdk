//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv)
{
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
    Qiniu_RS_EntryPathPair *entryPairs = (Qiniu_RS_EntryPathPair *)malloc(sizeof(Qiniu_RS_EntryPathPair) * entryCount);
    for (i = 0; i < entryCount; i++)
    {

        size_t indexLen = snprintf(NULL, 0, "%d", i) + 1;
        char *indexStr = (char *)calloc(sizeof(char), indexLen);
        snprintf(indexStr, indexLen, "%d", i);

        // src
        Qiniu_RS_EntryPath srcEntry;
        srcEntry.bucket = bucket;
        srcEntry.key = Qiniu_String_Concat2(key, indexStr);

        // dest
        Qiniu_RS_EntryPath destEntry;
        destEntry.bucket = bucket;
        destEntry.key = Qiniu_String_Concat3(key, indexStr, "-move");

        Qiniu_Free(indexStr);

        // pack
        Qiniu_RS_EntryPathPair entryPair;
        entryPair.src = srcEntry;
        entryPair.dest = destEntry;
        entryPair.force = Qiniu_True;
        entryPairs[i] = entryPair;
    }

    itemRets = (Qiniu_RS_BatchItemRet *)malloc(sizeof(Qiniu_RS_BatchItemRet) * entryCount);

    // init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_BatchMove(&client, itemRets, entryPairs, entryCount);
    if (error.code / 100 != 2)
    {
        printf("batch move file error.\n");
        debug_log(&client, error);
    }
    else
    {
        printf("batch move file success.\n\n");

        for (i = 0; i < entryCount; i++)
        {
            int code = itemRets[i].code;
            if (code == Qiniu_OK.code)
            {
                printf("success: %d\n", code);
            }
            else
            {
                printf("code: %d, error: %s\n", code, itemRets[i].error);
            }
        }
    }

    for (i = 0; i < entryCount; i++)
    {
        Qiniu_RS_EntryPathPair entryPair = entryPairs[i];
        Qiniu_Free((void *)entryPair.src.key);
        Qiniu_Free((void *)entryPair.dest.key);
    }

    Qiniu_Free(entryPairs);
    Qiniu_Free(itemRets);
    Qiniu_Client_Cleanup(&client);
}
