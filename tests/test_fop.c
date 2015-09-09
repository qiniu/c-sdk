#include "../qiniu/fop.h"
#include "test.h"
#include <stdio.h>

static void pfop(Qiniu_Client* client)
{
    Qiniu_Error err;
	Qiniu_FOP_PfopRet ret;
    Qiniu_FOP_PfopArgs args;
    char * fop[1];
    int fopCount = sizeof(fop) / sizeof(fop[0]);

    memset(&args, 0, sizeof(args));
    memset(&ret, 0, sizeof(ret));

    args.bucket = "csdk";
    args.key = "test_pfop.mp4";
    fop[0] = "avthumb/avi/vb/100k";
    err = Qiniu_FOP_Pfop(client, &ret, &args, fop, fopCount);
    CU_ASSERT(err.code == 200);
    CU_ASSERT(ret.persistentId != 0);
}

void testFop()
{
	Qiniu_Client client;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);

	pfop(&client);

	Qiniu_Client_Cleanup(&client);
}
