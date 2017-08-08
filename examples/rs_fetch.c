//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char**argv) {
    Qiniu_RS_FetchRet fetchRet;
    Qiniu_Client client;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *resURL="http://devtools.qiniu.com/qiniu.png";
    char *bucket = "csdk";
    char *key = "qiniu.png";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Client_InitMacAuth(&client, 1024, &mac);

    //fetch with key
    Qiniu_Error error1 = Qiniu_RS_Fetch(&client, &fetchRet,resURL, bucket, key);
    if (error1.code != 200) {
        printf("fetch file %s -> %s:%s error.\n",resURL, bucket,key);
        debug_log(&client, error1);
    } else {
        /*200, 正确返回了, 你可以通过fetchRet变量查询一些关于这个文件的信息*/
        printf("fetch file %s -> %s:%s success.\n",resURL, bucket,key);
        printf("file key: \t%s\n",fetchRet.key);
        printf("file hash: \t%s\n", fetchRet.hash);
        printf("file size: \t%lld\n", fetchRet.fsize);
        printf("file mime type: \t%s\n", fetchRet.mimeType);
    }

    //fetch without key
    Qiniu_Error error2 = Qiniu_RS_Fetch(&client, &fetchRet,resURL, bucket, 0);
    if (error2.code != 200) {
        printf("fetch file %s -> %s error.\n",resURL, bucket);
        debug_log(&client, error2);
    } else {
        /*200, 正确返回了, 你可以通过fetchRet变量查询一些关于这个文件的信息*/
        printf("fetch file %s -> %s success.\n",resURL, bucket);
        printf("file key: \t%s\n",fetchRet.key);
        printf("file hash: \t%s\n", fetchRet.hash);
        printf("file size: \t%lld\n", fetchRet.fsize);
        printf("file mime type: \t%s\n", fetchRet.mimeType);
    }


    Qiniu_Client_Cleanup(&client);
}