#include "../qiniu/cdn.h"
#include "debug.h"

int main(int argc, char **argv)
{
    Qiniu_Client client;
    Qiniu_CDN_FluxRet ret;
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
    error = Qiniu_CDN_GetFluxData(&client, &ret, startDate, endDate, g, domains, domainsCount);
    if (error.code != Qiniu_OK.code)
    {
        printf("get domain flux error.\n");
        debug_log(&client, error);
    }
    else
    {
        printf("get domain flux success.\n\n");
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
            Qiniu_CDN_FluxData fluxData = ret.data[i];
            if (fluxData.chinaCount == 0)
            {
                printf("China: no flux data\n");
            }
            else
            {
                printf("China: flux data\n");
                for (j = 0; j < fluxData.chinaCount; j++)
                {
                    printf("%lld\t", fluxData.china[j]);
                }
                printf("\n");
            }

            if (fluxData.overseaCount == 0)
            {
                printf("Oversea: no flux data\n");
            }
            else
            {
                printf("Oversea: flux data\n");
                for (j = 0; j < fluxData.overseaCount; j++)
                {
                    printf("%lld\t", fluxData.oversea[j]);
                }
                printf("\n");
            }
            printf("-----------\n");
        }

        Qiniu_Free_CDNFluxRet(&ret);
    }
    Qiniu_Client_Cleanup(&client);
}
