//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv)
{
    Qiniu_Global_Init(-1);

    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *key = "qiniu.png";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    // init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_Delete(&client, bucket, key);
    if (error.code != Qiniu_OK.code)
    {
        printf("delete file %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    }
    else
    {
        printf("delete file %s:%s file success.\n", bucket, key);
    }

    Qiniu_Client_Cleanup(&client);
}
