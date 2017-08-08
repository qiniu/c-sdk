#include "../qiniu/cdn.h"
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_Client client;
    Qiniu_CDN_RefreshRet ret;
    Qiniu_Error error;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    //urls to refresh
    char *urls[] = {
            "http://csdk.qiniudn.com/qiniu1.png",
            "http://csdk.qiniudn.com/qiniu2.png",
            "http://csdk.qiniudn.com/qiniu3.png"
    };

    //init
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    error = Qiniu_CDN_RefreshUrls(&client, &ret, urls, 3);
    if (error.code != 200) {
        printf("refresh urls error.\n");
        debug_log(&client, error);
    } else {
        printf("%d\n", ret.code);
        printf("%s\n", ret.error);
    }
}