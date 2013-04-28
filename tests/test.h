#include "../CUnit/CUnit/Headers/Basic.h"
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../qiniu/rs.h"

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
/* func Qiniu_IsEqual */

int Qiniu_IsEqual(Qiniu_Reader a, Qiniu_Reader b);

typedef struct _Qiniu_Eq {
	Qiniu_Reader v;
	int result;
} Qiniu_Eq;

int Qiniu_Is(Qiniu_Eq* eq);

Qiniu_Writer Qiniu_EqWriter(Qiniu_Eq* self, Qiniu_Reader v);

/*============================================================================*/
/* type Qiniu_Seq */

typedef struct _Qiniu_Seq {
	size_t off;
	size_t limit;
	int radix;	// 10
	int base;	// '0'
	size_t delta; // 0
} Qiniu_Seq;

Qiniu_Reader Qiniu_SeqReader(Qiniu_Seq* self, size_t limit, int radix, int base, size_t delta);
Qiniu_ReaderAt Qiniu_SeqReaderAt(Qiniu_Seq* self, size_t limit, int radix, int base, size_t delta);

/*============================================================================*/

