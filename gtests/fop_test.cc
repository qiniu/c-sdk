#include <gtest/gtest.h>
#include "test.h"
#include "qiniu/fop.h"

static void pfop(Qiniu_Client *client)
{
	Qiniu_Error err;
	Qiniu_FOP_PfopRet ret;
	memset(&ret, 0, sizeof(ret));

	char *fop[1];
	int fopCount = sizeof(fop) / sizeof(fop[0]);
	fop[0] = (char *)"avthumb/avi/vb/100k";

	const char *bucket = "csdk";
	const char *key = "test_pfop.mp4";
	err = Qiniu_FOP_Pfop(client, &ret, bucket, key, fop, fopCount, NULL, NULL, false);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_NE(ret.persistentId, nullptr);
}

TEST(IntegrationTest, TestFop)
{
	Qiniu_Client client;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	pfop(&client);

	Qiniu_Client_Cleanup(&client);
}
