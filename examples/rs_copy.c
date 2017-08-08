//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_RS_StatRet statRet;
    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *srcBucket = getenv("QINIU_TEST_BUCKET");;
    char *srcKey = "qiniu.png";
    char *destBucket = srcBucket;
    char *destKey = "qiniu_copy.png";
    Qiniu_Bool force = Qiniu_True;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_Copy(&client, srcBucket, srcKey, destBucket, destKey, force);
    if (error.code != 200) {
        printf("copy file %s:%s -> %s:%s error.\n", srcBucket, srcKey, destBucket, destKey);
        debug_log(&client, error);
    } else {
        printf("copy file %s:%s -> %s:%s success.\n", srcBucket, srcKey, destBucket, destKey);
    }

    Qiniu_Client_Cleanup(&client);
}