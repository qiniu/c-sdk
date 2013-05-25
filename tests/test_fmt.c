/*
 ============================================================================
 Name        : test_fmt.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void testFmt(void)
{
	Qiniu_Error err;

	char* p;
	p = Qiniu_String_Format(32, "%d4", -123);
	printf("%s\n", p);
	CU_ASSERT_STRING_EQUAL(p, "-1234");
	free(p);

	p = Qiniu_String_Format(32, "%d%%%D", 0, (Qiniu_Int64)-123);
	printf("%s\n", p);
	CU_ASSERT_STRING_EQUAL(p, "0%-123");
	free(p);

	p = Qiniu_String_Format(32, "0%U", (Qiniu_Uint64)123);
	printf("%s\n", p);
	CU_ASSERT_STRING_EQUAL(p, "0123");
	free(p);

	p = Qiniu_String_Format(32, "0%s", "123");
	printf("%s\n", p);
	CU_ASSERT_STRING_EQUAL(p, "0123");
	free(p);

	p = Qiniu_String_Format(32, "%S -> %s", "123", "4");
	printf("%s\n", p);
	CU_ASSERT_STRING_EQUAL(p, "MTIz -> 4");
	free(p);

	err.code = 400;
	err.message = "invalid arguments";
	p = Qiniu_String_Format(32, "[INFO] %E", err);
	Qiniu_Log_Warn("%E", err);
	CU_ASSERT_STRING_EQUAL(p, "[INFO] E400 invalid arguments");
	free(p);
}

