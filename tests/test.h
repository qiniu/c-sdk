#include "../CUnit/CUnit/Headers/Basic.h"
#include "../CUnit/CUnit/Headers/CUnit.h"

#define QBOX_TESTS_BEGIN(testClass)					\
	static CU_TestInfo tests_##testClass[] = {

#define QBOX_TEST(fnTest)							\
		{ #fnTest, fnTest },

#define QBOX_TESTS_END()							\
		CU_TEST_INFO_NULL							\
	};

#define QBOX_SUITES_BEGIN()							\
	static CU_SuiteInfo suites[] = {

#define QBOX_SUITE(testClass)						\
		{ #testClass, NULL, NULL, tests_##testClass },

#define QBOX_SUITE_EX(testClass, setUp, tearDown)	\
		{ #testClass, setUp, tearDown, tests_##testClass },

#define QBOX_SUITES_END()							\
		CU_SUITE_INFO_NULL,							\
	};

#define QBOX_ONE_SUITE_EX(testClass, setUp, tearDown) \
	QBOX_SUITES_BEGIN()								\
		QBOX_SUITE_EX(testClass, setUp, tearDown)	\
	QBOX_SUITES_END()

#define QBOX_ONE_SUITE(testClass)					\
	QBOX_ONE_SUITE_EX(testClass, NULL, NULL)

