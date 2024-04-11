//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include "../qiniu/conf.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv)
{
    Qiniu_RS_StatRet statRet;
    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");

    if (str_empty(accessKey) || str_empty(accessKey) || str_empty(bucket))
    {
        printf("please fill `test-env.sh` and then run `source test-env.sh` first\n");
        return -1;
    }

    char *key = "qiniu.png";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Use_Zone_Huadong(Qiniu_True);
    // init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_Stat(&client, &statRet, bucket, key);
    if (error.code != Qiniu_OK.code)
    {
        printf("stat file %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    }
    else
    {
        /*Qiniu_OK.code, 正确返回了, 你可以通过statRet变量查询一些关于这个文件的信息*/
        printf("stat file \t%s:%s success.\n\n", bucket, key);
        printf("file hash: \t%s\n", statRet.hash);
        printf("file size: \t%lld\n", statRet.fsize);
        printf("file put time: \t%lld\n", statRet.putTime);
        printf("file mime type: \t%s\n", statRet.mimeType);
        printf("file type: \t%lld\n", statRet.type);
    }

    Qiniu_Client_Cleanup(&client);
}
