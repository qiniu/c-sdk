#include <gtest/gtest.h>
#include "test.h"
#include "qiniu/rsf.h"

TEST(IntegrationTest, TestRsfListFiles)
{
    Qiniu_Client client;
    Qiniu_Error err;
    Qiniu_RSF_ListRet listRet;
    Qiniu_Zero(listRet);
    int totalCount = 0;

    Qiniu_Client_InitMacAuth(&client, 1024, NULL);
    Qiniu_Client_SetTimeout(&client, 5000);
    Qiniu_Client_SetConnectTimeout(&client, 3000);
    Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

    do
    {
        err = Qiniu_RSF_ListFiles(&client, &listRet, Test_bucket, "", "", listRet.marker, 1000);
        EXPECT_EQ(err.code, 200);
        for (int i = 0; i < listRet.itemsCount; i++)
        {
            printf("%s\n", listRet.items[i].key);
        }
        totalCount += listRet.itemsCount;
    } while (listRet.marker != NULL);
    EXPECT_GT(totalCount, 0);

    Qiniu_Client_Cleanup(&client);
}
