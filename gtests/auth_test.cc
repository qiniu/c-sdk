#include <gtest/gtest.h>
#include <curl/curl.h>
#include "test.h"
#include "qiniu/http.h"

static Qiniu_Bool match_header_prefix(Qiniu_Header *header, const char *expected_header)
{
    for (Qiniu_Header *cur_header = header; cur_header != NULL; cur_header = cur_header->next)
    {
        if (strncmp(cur_header->data, expected_header, strlen(expected_header)) == 0)
        {
            return Qiniu_True;
        }
    }
    return Qiniu_False;
}

TEST(UnitTest, TestQiniuMacAuthV2)
{
    Qiniu_Mac mac = {"ak", "sk"};
    Qiniu_Auth auth = Qiniu_MacAuth(&mac);
    Qiniu_Error err;
    Qiniu_Header *header = NULL;
    header = curl_slist_append(header, "X-Qiniu-: a");
    header = curl_slist_append(header, "X-Qiniu: b");
    header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded");
    const char *body = "{\"name\": \"test\"}";
    Qiniu_MacAuth_Enable_Qiniu_Timestamp_Signature();
    err = auth.itbl->AuthV2(&mac, "GET", &header, "http://upload.qiniup.com/", body, strlen(body));
    EXPECT_EQ(err.code, Qiniu_OK.code);
    EXPECT_EQ(match_header_prefix(header, "Authorization: Qiniu ak:"), Qiniu_True);
    EXPECT_EQ(match_header_prefix(header, "X-Qiniu-Date:"), Qiniu_True);
    curl_slist_free_all(header);

    header = NULL;
    Qiniu_MacAuth_Disable_Qiniu_Timestamp_Signature();
    err = auth.itbl->AuthV2(&mac, "GET", &header, "http://upload.qiniup.com/", body, strlen(body));
    Qiniu_MacAuth_Enable_Qiniu_Timestamp_Signature();
    EXPECT_EQ(err.code, Qiniu_OK.code);
    EXPECT_EQ(match_header_prefix(header, "X-Qiniu-Date:"), Qiniu_False);
    curl_slist_free_all(header);

    header = NULL;
    err = auth.itbl->AuthV2(&mac, "GET", &header, "http://upload.qiniup.com/", body, strlen(body));
    EXPECT_EQ(err.code, Qiniu_OK.code);
    EXPECT_EQ(match_header_prefix(header, "X-Qiniu-Date:"), Qiniu_True);
    curl_slist_free_all(header);

    header = NULL;
    header = curl_slist_append(header, "X-Qiniu-Aaa: CCC");
    header = curl_slist_append(header, "X-Qiniu-Bbb: AAA");
    header = curl_slist_append(header, "X-Qiniu-Bbb: BBB");
    header = curl_slist_append(header, "X-Qiniu-Aaa: DDD");
    body = "name=test&language=go}";
    Qiniu_MacAuth_Disable_Qiniu_Timestamp_Signature();
    err = auth.itbl->AuthV2(&mac, "GET", &header, "http://upload.qiniup.com/mkfile/sdf.jpg", body, strlen(body));
    Qiniu_MacAuth_Enable_Qiniu_Timestamp_Signature();
    EXPECT_EQ(err.code, Qiniu_OK.code);
    match_header_prefix(header, "Authorization: Qiniu ak:K8d62cW_hqjxQ3RElNz8g3BQHa8=");
    EXPECT_EQ(match_header_prefix(header, "X-Qiniu-Date:"), Qiniu_False);

    Qiniu_MacAuth_Disable_Qiniu_Timestamp_Signature();
    err = auth.itbl->AuthV2(&mac, "GET", &header, "http://upload.qiniup.com/mkfile/sdf.jpg?s=er3&df", body, strlen(body));
    Qiniu_MacAuth_Enable_Qiniu_Timestamp_Signature();
    EXPECT_EQ(err.code, Qiniu_OK.code);
    match_header_prefix(header, "Authorization: Qiniu ak:CzOiB_NSxrvMLkhK8hhV_1vTqYk=");
    EXPECT_EQ(match_header_prefix(header, "X-Qiniu-Date:"), Qiniu_False);
    curl_slist_free_all(header);
}
