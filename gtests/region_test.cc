#include <gtest/gtest.h>
#include "test.h"
#include "qiniu/region.h"
#include "qiniu/private/region.h"

#ifdef _WIN32
#include <windows.h>
#endif

TEST(UnitTest, TestRegion)
{
    size_t count;
    const char *const *hosts;

    const char *const upPreferredHosts[] = {
        "up.qiniup.com",
        "upload.qiniup.com",
    };
    const char *const upAlternativeHosts[] = {
        "up.qbox.me",
    };
    const char *const ioPreferredHosts[] = {
        "https://iovip.qiniuio.com",
    };
    const char *const ioAlternativeHosts[] = {
        "http://iovip.qbox.me",
    };

    Qiniu_Region *region = Qiniu_Region_New("z0", 1);
    EXPECT_STREQ(Qiniu_Region_Get_Id(region), "z0");
#ifdef _WIN32
    Sleep(2000);
#else
    sleep(2);
#endif
    EXPECT_TRUE(Qiniu_Region_Is_Expired(region));

    Qiniu_Region_Set_Up_Preferred_Hosts(region, upPreferredHosts, 2, Qiniu_True);
    Qiniu_Region_Set_Up_Alternative_Hosts(region, upAlternativeHosts, 1, Qiniu_False);
    Qiniu_Region_Set_Io_Preferred_Hosts(region, ioPreferredHosts, 1, Qiniu_False);
    Qiniu_Region_Set_Io_Alternative_Hosts(region, ioAlternativeHosts, 1, Qiniu_True);

    hosts = Qiniu_Region_Get_Up_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(hosts[0], "https://up.qiniup.com");
    EXPECT_STREQ(hosts[1], "https://upload.qiniup.com");

    hosts = Qiniu_Region_Get_Up_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://up.qbox.me");

    hosts = Qiniu_Region_Get_Io_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://iovip.qiniuio.com");

    hosts = Qiniu_Region_Get_Io_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://iovip.qbox.me");

    hosts = Qiniu_Region_Get_Io_Src_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(hosts == NULL);

    hosts = Qiniu_Region_Get_Io_Src_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(hosts == NULL);

    Qiniu_Region_Free(region);
}

TEST(UnitTest, TestUseRegion)
{
    size_t count;
    const char *const *hosts;

    Qiniu_Region *region = Qiniu_Use_Region("z0", Qiniu_True);

    hosts = Qiniu_Region_Get_Up_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(hosts[0], "https://upload.qiniup.com");
    EXPECT_STREQ(hosts[1], "https://up.qiniup.com");

    hosts = Qiniu_Region_Get_Up_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://up.qbox.me");

    hosts = Qiniu_Region_Get_Io_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://iovip.qiniuio.com");

    hosts = Qiniu_Region_Get_Io_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://iovip.qbox.me");

    hosts = Qiniu_Region_Get_Rs_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://rs-z0.qiniuapi.com");

    hosts = Qiniu_Region_Get_Rs_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://rs-z0.qbox.me");

    hosts = Qiniu_Region_Get_Rsf_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://rsf-z0.qiniuapi.com");

    hosts = Qiniu_Region_Get_Rsf_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://rsf-z0.qbox.me");

    hosts = Qiniu_Region_Get_Api_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://api-z0.qiniuapi.com");

    hosts = Qiniu_Region_Get_Api_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://api-z0.qbox.me");

    Qiniu_Region_Free(region);

    region = Qiniu_Use_Region("z1", Qiniu_False);

    hosts = Qiniu_Region_Get_Up_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(hosts[0], "http://upload-z1.qiniup.com");
    EXPECT_STREQ(hosts[1], "http://up-z1.qiniup.com");

    hosts = Qiniu_Region_Get_Up_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://up-z1.qbox.me");

    hosts = Qiniu_Region_Get_Io_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://iovip-z1.qiniuio.com");

    hosts = Qiniu_Region_Get_Io_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://iovip-z1.qbox.me");

    hosts = Qiniu_Region_Get_Rs_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://rs-z1.qiniuapi.com");

    hosts = Qiniu_Region_Get_Rs_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://rs-z1.qbox.me");

    hosts = Qiniu_Region_Get_Rsf_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://rsf-z1.qiniuapi.com");

    hosts = Qiniu_Region_Get_Rsf_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://rsf-z1.qbox.me");

    hosts = Qiniu_Region_Get_Api_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://api-z1.qiniuapi.com");

    hosts = Qiniu_Region_Get_Api_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "http://api-z1.qbox.me");

    Qiniu_Region_Free(region);
}

TEST(IntegrationTest, TestRegionQuery)
{
    Qiniu_Client client;
    Qiniu_Client_InitMacAuth(&client, 1024, NULL);
    Qiniu_Client_SetTimeout(&client, 5000);
    Qiniu_Client_SetConnectTimeout(&client, 3000);

    Qiniu_Region *region;
    Qiniu_Error err = Qiniu_Region_Query(&client, &region, Test_bucket, Qiniu_True);
    EXPECT_EQ(err.code, 200);

    EXPECT_STREQ(Qiniu_Region_Get_Id(region), "z0");

    size_t count;
    const char *const *hosts;

    hosts = Qiniu_Region_Get_Up_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(hosts[0], "https://upload.qiniup.com");
    EXPECT_STREQ(hosts[1], "https://up.qiniup.com");

    hosts = Qiniu_Region_Get_Up_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(hosts[0], "https://upload.qbox.me");
    EXPECT_STREQ(hosts[1], "https://up.qbox.me");

    hosts = Qiniu_Region_Get_Io_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://iovip.qbox.me");

    hosts = Qiniu_Region_Get_Io_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(hosts == NULL);

    hosts = Qiniu_Region_Get_Io_Src_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_TRUE(hosts != NULL);

    hosts = Qiniu_Region_Get_Io_Src_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(hosts == NULL);

    hosts = Qiniu_Region_Get_Rs_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://rs-z0.qbox.me");

    hosts = Qiniu_Region_Get_Rs_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(hosts == NULL);

    hosts = Qiniu_Region_Get_Rsf_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://rsf-z0.qbox.me");

    hosts = Qiniu_Region_Get_Rsf_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(hosts == NULL);

    hosts = Qiniu_Region_Get_Api_Preferred_Hosts(region, &count);
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(hosts[0], "https://api.qiniu.com");

    hosts = Qiniu_Region_Get_Api_Alternative_Hosts(region, &count);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(hosts == NULL);

    Qiniu_Region_Free(region);
}

TEST(IntegrationTest, TestRegionAutoQuery)
{
    Qiniu_Client client;
    Qiniu_Client_InitMacAuth(&client, 1024, NULL);
    Qiniu_Client_SetTimeout(&client, 5000);
    Qiniu_Client_SetConnectTimeout(&client, 3000);
    Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

    const char *host;
    Qiniu_Error err;

    err = _Qiniu_Region_Get_Up_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "https://upload.qiniup.com");

    err = _Qiniu_Region_Get_Io_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "https://iovip.qbox.me");

    err = _Qiniu_Region_Get_Rs_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "https://rs-z0.qbox.me");

    err = _Qiniu_Region_Get_Rsf_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "https://rsf-z0.qbox.me");

    err = _Qiniu_Region_Get_Api_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "https://api.qiniu.com");
}

TEST(IntegrationTest, TestRegionSpecify)
{
    Qiniu_Client client;
    Qiniu_Client_InitMacAuth(&client, 1024, NULL);
    Qiniu_Client_SetTimeout(&client, 5000);
    Qiniu_Client_SetConnectTimeout(&client, 3000);

    Qiniu_Region *region = Qiniu_Use_Region("z1", Qiniu_False);
    Qiniu_Client_SpecifyRegion(&client, region);

    const char *host;
    Qiniu_Error err;

    err = _Qiniu_Region_Get_Up_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "http://upload-z1.qiniup.com");

    err = _Qiniu_Region_Get_Io_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "http://iovip-z1.qiniuio.com");

    err = _Qiniu_Region_Get_Rs_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "http://rs-z1.qiniuapi.com");

    err = _Qiniu_Region_Get_Rsf_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "http://rsf-z1.qiniuapi.com");

    err = _Qiniu_Region_Get_Api_Host(&client, NULL, Test_bucket, &host);
    EXPECT_EQ(err.code, 200);
    EXPECT_STREQ(host, "http://api-z1.qiniuapi.com");

    Qiniu_Region_Free(region);
}
