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
    char *delimiter = 0;
    char *marker = 0;
    int limit = 100;

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RSF_ListFiles(&client, &listRet, bucket, prefix, delimiter, marker, limit);
    if (error.code != 200) {
        printf("list files of bucket %s error.\n", bucket);
        debug_log(&client, error);
    } else {
        /*200, 正确返回了, 你可以通过listRet变量查询文件列表信息*/
        printf("list files of bucket %s success.\n\n", bucket);

    }

    Qiniu_Client_Cleanup(&client);
}