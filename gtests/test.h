#ifndef QINIU_TEST_H
#define QINIU_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "qiniu/base.h"

#ifdef _WIN32
#define TEMP_DIR "C:\\Windows\\Temp"
#else
#define TEMP_DIR "/tmp"
#endif

	/*============================================================================*/
	/* func Qiniu_IsEqual */

	int Qiniu_IsEqual(Qiniu_Reader a, Qiniu_Reader b);

	typedef struct _Qiniu_Eq
	{
		Qiniu_Reader v;
		int result;
	} Qiniu_Eq;

	int Qiniu_Is(Qiniu_Eq *eq);

	Qiniu_Writer Qiniu_EqWriter(Qiniu_Eq *self, Qiniu_Reader v);

	/*============================================================================*/
	/* type Qiniu_Seq */

	typedef struct _Qiniu_Seq
	{
		size_t off;
		size_t limit;
		int radix;    // 10
		int base;     // '0'
		size_t delta; // 0
	} Qiniu_Seq;

	Qiniu_Reader Qiniu_SeqReader(Qiniu_Seq *self, size_t limit, int radix, int base, size_t delta);
	Qiniu_ReaderAt Qiniu_SeqReaderAt(Qiniu_Seq *self, size_t limit, int radix, int base, size_t delta);

	/*============================================================================*/

	extern const char *Test_bucket;
	extern const char *Test_Domain;

#ifdef __cplusplus
}
#endif

#endif
