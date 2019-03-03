#include "../qiniu/rs.h"
#include <stdlib.h>

int main(int argc, char **argv) {
    Qiniu_Global_Init(-1);

    Qiniu_Client client;

    char *accessKey = "<ak>";
    char *secretKey = "<sk>";
    char *bucketName = "<bucket>";
    char *region = "<region>";

    Qiniu_Mac mac;
    mac.accessKey = accessKey;
    mac.secretKey = secretKey;

    Qiniu_Client_InitMacAuth(&client, 1024, &mac);
    Qiniu_Error error = Qiniu_RS_CreateBucket(&client, bucketName, region);

    if (error.code != 200) {
        printf("%d\n", error.code);
        printf("%s\n", error.message);
    } else {
        printf("%d\n", error.code);
    }

    Qiniu_Client_Cleanup(&client);
}
