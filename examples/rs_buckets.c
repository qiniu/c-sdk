#include "../qiniu/rs.h"
#include <stdlib.h>

int main(int argc, char **argv) {
    Qiniu_Global_Init(-1);

    Qiniu_Client client;
    Qiniu_RS_BucketsRet *ret = (Qiniu_RS_BucketsRet*) malloc(sizeof(Qiniu_RS_BucketsRet));

    char *accessKey = "<ak>";
    char *secretKey = "<sk>";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_Buckets(&client, ret);
    
    if (error.code != 200) {
        printf("%d\n", error.code);
    } else {
        printf("%d\n", error.code);
        while (ret != NULL) {
            printf("%s\n", ret->bucket);
            ret = ret->next;
        }
    }

    Qiniu_Client_Cleanup(&client);
}
