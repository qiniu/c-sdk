#include <stdio.h>
#include "../qiniu/base.h"

/*debug 函数*/
void debug_log(Qiniu_Client *client, Qiniu_Error err) {
    printf("\nerror code: %d, message: %s\n", err.code, err.message);
    printf("response header:\n%s", Qiniu_Buffer_CStr(&client->respHeader));
    printf("response body:\n%s", Qiniu_Buffer_CStr(&client->b));
    printf("\n\n\n");
}