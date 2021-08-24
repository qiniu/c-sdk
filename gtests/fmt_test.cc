#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

TEST(UnitTest, TestFmt)
{
	Qiniu_Error err;

	char *p;
	p = Qiniu_String_Format(32, "%d4", -123);
	Qiniu_Log_Debug("%s", p);
	EXPECT_STREQ(p, "-1234");
	free(p);

	p = Qiniu_String_Format(32, "%d%%%D", 0, (Qiniu_Int64)-123);
	Qiniu_Log_Debug("%s", p);
	EXPECT_STREQ(p, "0%-123");
	free(p);

	p = Qiniu_String_Format(32, "0%U", (Qiniu_Uint64)123);
	Qiniu_Log_Debug("%s", p);
	EXPECT_STREQ(p, "0123");
	free(p);

	p = Qiniu_String_Format(32, "0%s", "123");
	Qiniu_Log_Debug("%s", p);
	EXPECT_STREQ(p, "0123");
	free(p);

	p = Qiniu_String_Format(32, "%S -> %s", "123", "4");
	Qiniu_Log_Debug("%s", p);
	EXPECT_STREQ(p, "MTIz -> 4");
	free(p);

	err.code = 400;
	err.message = "invalid arguments";
	p = Qiniu_String_Format(32, "[INFO] %E", err);
	Qiniu_Log_Warn("%E", err);
	EXPECT_STREQ(p, "[INFO] E400 invalid arguments");
	free(p);
}
