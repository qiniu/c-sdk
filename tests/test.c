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

void testFileIo();
void testBaseIo();
void testIoPut();
void testResumableIoPut();
void testFmt();
void testEqual();
void testRsBatchOps();
void testFop();
void testMultipartUpload_smallfile();
void testMultipartUpload_largefile();
void testMultipartUpload_emptyfile();
void testMultipartUpload_inMemoryData();

static int setup()
{
	printf("setup\n");
	return 0;
}

static int teardown()
{
	printf("teardown\n");
	return 0;
}

int main()
{
	Qiniu_Servend_Init(0);

	QINIU_ACCESS_KEY = getenv("QINIU_ACCESS_KEY");
	QINIU_SECRET_KEY = getenv("QINIU_SECRET_KEY");

	assert(QINIU_ACCESS_KEY != NULL);
	assert(QINIU_SECRET_KEY != NULL);
	CU_pSuite pSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pSuite = CU_add_suite("qiniu", setup, teardown);
	if (NULL == pSuite)
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* add the tests to the suite */
	CU_add_test(pSuite, "testFmt", testFmt);
	// CU_add_test(pSuite, "testBaseIo", testBaseIo);
	// CU_add_test(pSuite, "testFileIo", testFileIo);
	// CU_add_test(pSuite, "testEqual", testEqual);
	// CU_add_test(pSuite, "testResumableIoPut", testResumableIoPut);
	CU_add_test(pSuite, "testMultipartUpload_smallfile", testMultipartUpload_smallfile);
	CU_add_test(pSuite, "testMultipartUpload_largefile", testMultipartUpload_largefile);
	CU_add_test(pSuite, "testMultipartUpload_emptyfile", testMultipartUpload_emptyfile);
	CU_add_test(pSuite, "testMultipartUpload_inMemoryData", testMultipartUpload_inMemoryData);
	// CU_add_test(pSuite, "testIoPut", testIoPut);
	// CU_add_test(pSuite, "testRsBatchOps", testRsBatchOps);
	// CU_add_test(pSuite, "testFop", testFop);

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
