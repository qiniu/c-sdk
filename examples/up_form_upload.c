//
// Created by jemy on 07/08/2017.
//

#include "../qiniu/io.h"
#include "../qiniu/rs.h"
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_Global_Init(-1);

    Qiniu_Io_PutRet putRet;
    Qiniu_Client client;
    Qiniu_RS_PutPolicy putPolicy;
    Qiniu_Io_PutExtra putExtra;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");
    char *bucket = getenv("QINIU_TEST_BUCKET");
    char *key = "qiniu.mp4";
    char *localFile = "/tmp/qiniu.mp4";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Zero(putPolicy);
    Qiniu_Zero(putExtra);

    putPolicy.scope = bucket;
    char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);

    //设置机房域名
    //Qiniu_Use_Zone_Beimei(Qiniu_False);
    //Qiniu_Use_Zone_Huabei(Qiniu_True);
    Qiniu_Use_Zone_Huadong(Qiniu_False);
    //Qiniu_Use_Zone_Huanan(Qiniu_True);

    //put extra
    //putExtra.upHost="http://nbxs-gate-up.qiniu.com";

    //put extra
    // const char* ips[1] = {"124.160.115.130"};
    // putExtra.upIps = ips;
    // putExtra.ipCount = 1;

    Qiniu_Io_PutExtraParam metaParam_1;
    Qiniu_Zero(metaParam_1);
    Qiniu_Io_PutExtraParam metaParam_2;
    Qiniu_Zero(metaParam_2);

    putExtra.params = &metaParam_1;
    metaParam_1.key = "x-qn-meta-Meta-Key-1";
    metaParam_1.value = "x-qn-meta-Meta-Value-1";
    metaParam_1.next = &metaParam_2;
    metaParam_2.key = "x-qn-meta-Meta-Key-2";
    metaParam_2.value = "x-qn-meta-Meta-Value-2";
    metaParam_2.next = NULL;

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_Io_PutFile(&client, &putRet, uptoken, key, localFile, &putExtra);
    if (error.code != Qiniu_OK.code) {
        printf("upload file %s:%s error.\n", bucket, key);
        debug_log(&client, error);
    } else {
        /*Qiniu_OK.code, 正确返回了, 你可以通过statRet变量查询一些关于这个文件的信息*/
        printf("upload file %s:%s success.\n\n", bucket, key);
        printf("key:\t%s\n",putRet.key);
        printf("hash:\t%s\n", putRet.hash);
    }

    Qiniu_Free(uptoken);
    Qiniu_Client_Cleanup(&client);
}
