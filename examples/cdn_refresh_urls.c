#include "../qiniu/cdn.h"
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_Client client;
    Qiniu_CDN_RefreshRet ret;
    Qiniu_Error error;
    char **p;
    int i;

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
    Qiniu_Zero(ret);
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    error = Qiniu_CDN_RefreshUrls(&client, &ret, urls, 3);
    if (error.code != 200) {
        printf("refresh urls error.\n");
        debug_log(&client, error);
    } else {
        printf("refresh urls success.\n");
        printf("Code: %d\n", ret.code);
        printf("Error: %s\n", ret.error);
        printf("RequestId: %s\n", ret.requestId);

        p = ret.invalidUrls;
        for (i = 0; i < ret.invalidUrlsCount; i++) {
            printf("InvalidUrl: %s\n", *p);
            ++p;
        }

        p = ret.invalidDirs;
        for (i = 0; i < ret.invalidDirsCount; i++) {
            printf("InvalidDir: %s\n", *p);
            ++p;
        }

        printf("UrlQuotaDay: %d\n", ret.urlQuotaDay);
        printf("UrlSurplusDay: %d\n", ret.urlSurplusDay);
        printf("DirQuotaDay: %d\n", ret.dirQuotaDay);
        printf("DirSurplusDay: %d\n", ret.dirSurplusDay);

        Qiniu_Free_CDNRefreshRet(&ret);
    }
    Qiniu_Client_Cleanup(&client);
}