//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_Global_Init(-1);

    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *key = "qiniu.png";
    int fileType = 1;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_ChangeType(&client, bucket, key, fileType);
    if (error.code != 200) {
        printf("change file type %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    } else {
        printf("change file type %s:%s success.\n\n", bucket, key);
    }

    Qiniu_Client_Cleanup(&client);
}