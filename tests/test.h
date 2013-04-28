#include "../CUnit/CUnit/Headers/Basic.h"
#include "../CUnit/CUnit/Headers/CUnit.h"

/*============================================================================*/

#define QINIU_TESTS_BEGIN(testClass)				\
	static CU_TestInfo tests_##testClass[] = {

#define QINIU_TEST(fnTest)							\
		{ #fnTest, fnTest },

#define QINIU_TESTS_END()							\
		CU_TEST_INFO_NULL							\
	};

#define QINIU_SUITES_BEGIN()						\
	static CU_SuiteInfo suites[] = {

#define QINIU_SUITE(testClass)						\
		{ #testClass, NULL, NULL, tests_##testClass },

#define QINIU_SUITE_EX(testClass, setUp, tearDown)	\
		{ #testClass, setUp, tearDown, tests_##testClass },

#define QINIU_SUITES_END()							\
		CU_SUITE_INFO_NULL,							\
	};

#define QINIU_ONE_SUITE_EX(testClass, setUp, tearDown) \
	QINIU_SUITES_BEGIN()							\
		QINIU_SUITE_EX(testClass, setUp, tearDown)	\
	QINIU_SUITES_END()

#define QINIU_ONE_SUITE(testClass)					\
	QINIU_ONE_SUITE_EX(testClass, NULL, NULL)

/*============================================================================*/


/*============================================================================*/

