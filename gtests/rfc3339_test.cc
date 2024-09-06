#include <gtest/gtest.h>
#include "test.h"
#include "qiniu/rfc3339.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    void _Qiniu_Parse_Date_Time(char *datetime_string, Qiniu_DateTime *dt);
#ifdef __cplusplus
}
#endif

TEST(UnitTest, TestQiniuParseDateTime)
{
    Qiniu_DateTime dt;
    Qiniu_Zero(dt);
    _Qiniu_Parse_Date_Time((char *)"1937-02-02T12:00:27.87+00:20", &dt);
    EXPECT_TRUE(dt.ok);
    EXPECT_TRUE(dt.date.ok);
    EXPECT_EQ(dt.date.year, 1937);
    EXPECT_EQ(dt.date.month, 2);
    EXPECT_EQ(dt.date.day, 2);
    EXPECT_TRUE(dt.time.ok);
    EXPECT_EQ(dt.time.hour, 12);
    EXPECT_EQ(dt.time.minute, 0);
    EXPECT_EQ(dt.time.second, 27);
    EXPECT_EQ(dt.time.fraction, 870000);
    EXPECT_EQ(dt.time.offset, 20);

    _Qiniu_Parse_Date_Time((char *)"1937-02-29T12:00:27.87+00:20", &dt);
    EXPECT_FALSE(dt.ok);

    _Qiniu_Parse_Date_Time((char *)"1936-02-29T12:00:27.87Z", &dt);
    EXPECT_TRUE(dt.ok);
    EXPECT_TRUE(dt.date.ok);
    EXPECT_EQ(dt.date.year, 1936);
    EXPECT_EQ(dt.date.month, 2);
    EXPECT_EQ(dt.date.day, 29);
    EXPECT_TRUE(dt.time.ok);
    EXPECT_EQ(dt.time.hour, 12);
    EXPECT_EQ(dt.time.minute, 0);
    EXPECT_EQ(dt.time.second, 27);
    EXPECT_EQ(dt.time.fraction, 870000);
    EXPECT_EQ(dt.time.offset, 0);

    _Qiniu_Parse_Date_Time((char *)"1900-02-29T12:00:27.87+00:20", &dt);
    EXPECT_FALSE(dt.ok);
}
