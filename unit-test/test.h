/*
 ============================================================================
 Name        : qbox_test.h
 Author      : Wu Shi Yu
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */

#ifndef C_UNIT_TEST_MAIN
#define C_UNIT_TEST_MAIN

#define GENERATE_XML 1
#define ADD_BAD_TEST 2
static int myMode;

#include "../CUnit/CUnit/Headers/Basic.h"
#include "../CUnit/CUnit/Headers/CUnit.h"

#define QBOX_TESTS_BEGIN(testClass) \
static CU_TestInfo tests_##testClass[] = {

#define QBOX_TEST(fnTest) \
{ #fnTest, fnTest },

#define QBOX_TESTS_END() \
CU_TEST_INFO_NULL \
};

#define QBOX_SUITES_BEGIN() \
static CU_SuiteInfo suites[] = {

#define QBOX_SUITE(testClass) \
{ #testClass, NULL, NULL, tests_##testClass },

#define QBOX_SUITE_EX(testClass, setUp, tearDown) \
{ #testClass, setUp, tearDown, tests_##testClass },

#define QBOX_SUITES_END() \
CU_SUITE_INFO_NULL, \
};

#define QBOX_ONE_SUITE_EX(testClass, setUp, tearDown) \
QBOX_SUITES_BEGIN() \
QBOX_SUITE_EX(testClass, setUp, tearDown) \
QBOX_SUITES_END()

#define QBOX_ONE_SUITE(testClass) \
QBOX_ONE_SUITE_EX(testClass, NULL, NULL)

#define QBOX_TEST_REGISTE(suits_name) \
assert(NULL != CU_get_registry()); \
assert(!CU_is_test_running()); \
if(CUE_SUCCESS != CU_register_suites(suites)){  \
    fprintf(stderr, "Register suits_name failed - %s ", CU_get_error_msg()); \
    exit(EXIT_FAILURE); \
}
#endif

