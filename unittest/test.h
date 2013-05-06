/*
 ============================================================================
 Name        : QINIU_test.h
 Author      : Wu Shi Yu
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QINIU TEST
 ============================================================================
 */

#ifndef C_UNIT_TEST_MAIN
#define C_UNIT_TEST_MAIN

#define GENERATE_XML 1
#define ADD_BAD_TEST 2
static int myMode;

#include "../CUnit/CUnit/Headers/Basic.h"
#include "../CUnit/CUnit/Headers/CUnit.h"

#define QINIU_TESTS_BEGIN(testClass) \
static CU_TestInfo tests_##testClass[] = {

#define QINIU_TEST(fnTest) \
{ #fnTest, fnTest },

#define QINIU_TESTS_END() \
CU_TEST_INFO_NULL \
};

#define QINIU_SUITES_BEGIN() \
static CU_SuiteInfo suites[] = {

#define QINIU_SUITE(testClass) \
{ #testClass, NULL, NULL, tests_##testClass },

#define QINIU_SUITE_EX(testClass, setUp, tearDown) \
{ #testClass, setUp, tearDown, tests_##testClass },

#define QINIU_SUITES_END() \
CU_SUITE_INFO_NULL, \
};

#define QINIU_ONE_SUITE_EX(testClass, setUp, tearDown) \
QINIU_SUITES_BEGIN() \
QINIU_SUITE_EX(testClass, setUp, tearDown) \
QINIU_SUITES_END()

#define QINIU_ONE_SUITE(testClass) \
QINIU_ONE_SUITE_EX(testClass, NULL, NULL)

#define QINIU_TEST_REGISTE(suits_name) \
assert(NULL != CU_get_registry()); \
assert(!CU_is_test_running()); \
if(CUE_SUCCESS != CU_register_suites(suites)){  \
    fprintf(stderr, "Register suits_name failed - %s ", CU_get_error_msg()); \
    exit(EXIT_FAILURE); \
}
#endif

