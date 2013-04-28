/*
 ============================================================================
 Name        : test.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void testBaseIo();
void testIoPut();
void testResumableIoPut();
void testFmt();

QINIU_TESTS_BEGIN(qbox)
	QINIU_TEST(testFmt)
	QINIU_TEST(testBaseIo)
	QINIU_TEST(testResumableIoPut)
	QINIU_TEST(testIoPut)
QINIU_TESTS_END()

QINIU_ONE_SUITE(qbox)

int main()
{
	int err = 0;

	QINIU_ACCESS_KEY	= getenv("QINIU_ACCESS_KEY");
	QINIU_SECRET_KEY	= getenv("QINIU_SECRET_KEY");

	assert(QINIU_ACCESS_KEY != NULL);
	assert(QINIU_SECRET_KEY != NULL);

	Qiniu_Global_Init(-1);

	CU_initialize_registry();

	assert(NULL != CU_get_registry());
	assert(!CU_is_test_running());
	if (CU_register_suites(suites) != CUE_SUCCESS) {
		exit(EXIT_FAILURE);
	}

	CU_basic_set_mode(CU_BRM_NORMAL);
	CU_set_error_action(CUEA_FAIL);
	err = CU_basic_run_tests();
	CU_cleanup_registry();

	Qiniu_Global_Cleanup();
	return err;
}

