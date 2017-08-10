#include "../qiniu/cdn.h"
#include "debug.h"

int main(int argc, char **argv) {
    Qiniu_Client client;
    Qiniu_CDN_PrefetchRet ret;
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
    error = Qiniu_CDN_PrefetchUrls(&client, &ret, urls, 3);
    if (error.code != 200) {
        printf("prefetch urls error.\n");
        debug_log(&client, error);
    } else {
        printf("prefetch urls success.\n");
        printf("Code: %d\n", ret.code);
        printf("Error: %s\n", ret.error);
        printf("RequestId: %s\n", ret.requestId);

        p = ret.invalidUrls;
        for (i = 0; i < ret.invalidUrlsCount; i++) {
            printf("InvalidUrl: %s\n", *p);
            ++p;
        }

        printf("QuotaDay: %d\n", ret.quotaDay);
        printf("SurplusDay: %d\n", ret.surplusDay);

        Qiniu_Free_CDNPrefetchRet(&ret);
    }
    Qiniu_Client_Cleanup(&client);
}