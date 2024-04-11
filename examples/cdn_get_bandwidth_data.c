#include "../qiniu/cdn.h"
#include "debug.h"

int main(int argc, char **argv)
{
    Qiniu_Client client;
    Qiniu_CDN_BandwidthRet ret;
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
    char *startDate = "2017-08-01";
    char *endDate = "2017-08-09";
    char *g = "day";

    // init
    Qiniu_Zero(ret);
    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    error = Qiniu_CDN_GetBandwidthData(&client, &ret, startDate, endDate, g, domains, domainsCount);
    if (error.code != Qiniu_OK.code)
    {
        printf("get domain bandwidth error.\n");
        debug_log(&client, error);
    }
    else
    {
        printf("get domain bandwidth success.\n\n");
        printf("Code: %d\n", ret.code);
        printf("Error: %s\n", ret.error);
        printf("Domains: %d\n", ret.domainsCount);

        printf("-----------\n");

        for (i = 0; i < ret.timeCount; i++)
        {
            printf("%s\t", ret.time[i]);
        }
        printf("\n");

        // data
        for (i = 0; i < ret.domainsCount; i++)
        {
            printf("Domain:%s\n", domains[i]);
            Qiniu_CDN_BandwidthData bandData = ret.data[i];
            if (bandData.chinaCount == 0)
            {
                printf("China: no bandwidth data\n");
            }
            else
            {
                printf("China: bandwidth data\n");
                for (j = 0; j < bandData.chinaCount; j++)
                {
                    printf("%lld\t", bandData.china[j]);
                }
                printf("\n");
            }

            if (bandData.overseaCount == 0)
            {
                printf("Oversea: no bandwidth data\n");
            }
            else
            {
                printf("Oversea: bandwidth data\n");
                for (j = 0; j < bandData.overseaCount; j++)
                {
                    printf("%lld\t", bandData.oversea[j]);
                }
                printf("\n");
            }
            printf("-----------\n");
        }

        Qiniu_Free_CDNBandwidthRet(&ret);
    }
    Qiniu_Client_Cleanup(&client);
}
