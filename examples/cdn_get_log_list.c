#include "../qiniu/cdn.h"
#include "debug.h"

int main(int argc, char **argv)
{
    Qiniu_Client client;
    Qiniu_CDN_LogListRet ret;
    Qiniu_Error error;
    int i, j;

    char *accessKey = getenv("QINIU_ACCESS_KEY");
    char *secretKey = getenv("QINIU_SECRET_KEY");

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    // urls to refresh
    char *domains[] = {
        "csdk.qiniudn.com",
        "javasdk.qiniudn.com",
    };
    int domainsCount = 2;
    char *day = "2017-08-07";

    // init
    Qiniu_Zero(ret);
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    error = Qiniu_CDN_GetLogList(&client, &ret, domains, domainsCount, day);
    if (error.code != Qiniu_OK.code)
    {
        printf("get domain logs error.\n");
        debug_log(&client, error);
    }
    else
    {
        printf("get domain logs success.\n\n");
        printf("Code: %d\n", ret.code);
        printf("Error: %s\n", ret.error);
        printf("Domains: %d\n", ret.domainsCount);

        printf("-----------\n");

        // data
        for (i = 0; i < ret.domainsCount; i++)
        {
            Qiniu_CDN_LogListData logData = ret.data[i];
            if (logData.itemsCount == 0)
            {
                printf("domain: %s, no log data\n", logData.domain);
                printf("-----------\n");
            }
            else
            {
                printf("domain: %s have %d log files\n", logData.domain, logData.itemsCount);
                for (j = 0; j < logData.itemsCount; j++)
                {
                    printf("name: %s\n", logData.items[j].name);
                }
                printf("-----------\n");
            }
        }

        Qiniu_Free_CDNLogListRet(&ret);
    }
    Qiniu_Client_Cleanup(&client);
}
