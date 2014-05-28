/*
 ============================================================================
 Name        : test_fmt.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"

static const Qiniu_Int64 fsize = 4*1024*1024 + 2;

void testEqual(void)
{
	Qiniu_Eq eq;
	Qiniu_Seq seq, seq2;
	Qiniu_Section sect;

	Qiniu_ReaderAt in = Qiniu_SeqReaderAt(&seq, (size_t)fsize, 10, '0', 0);
	Qiniu_Reader r = Qiniu_SectionReader(&sect, in, 0, (off_t)fsize);

	Qiniu_Reader in2 = Qiniu_SeqReader(&seq2, (size_t)fsize, 10, '0', 0);
	Qiniu_Writer w = Qiniu_EqWriter(&eq, in2);

	Qiniu_Int64 ncopy;
	Qiniu_Error err = Qiniu_Copy(w, r, NULL, 1024, &ncopy);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(ncopy == fsize);
	CU_ASSERT(Qiniu_Is(&eq));
}

void testFileIo()
{
	char buf[24];
	size_t len = 20;
	size_t n;
	Qiniu_File* fp;

	Qiniu_Error err = Qiniu_File_Open(&fp, __FILE__);
	CU_ASSERT(err.code == 200);

	buf[len] = '\0';

	n = Qiniu_File_ReadAt(fp, buf, len, 0);
	printf("%s, %d, %d, %d\n", buf, n, len, errno);
	CU_ASSERT_EQUAL(n, len);

	n = Qiniu_File_ReadAt(fp, buf, len, 2);
	printf("%s, %d, %d, %d\n", buf, n, len, errno);
	CU_ASSERT_EQUAL(n, len);

	Qiniu_File_Close(fp);
}

void testBaseIo()
{
	char buf[32];
	size_t n;
	Qiniu_Seq seq;
	Qiniu_Crc32 crc32;
	Qiniu_Tee tee;
	Qiniu_Section sect;
	Qiniu_Reader in, in2;
	Qiniu_ReaderAt in3;
	Qiniu_Writer h;
	Qiniu_Error err;

	in = Qiniu_SeqReader(&seq, 12, 10, '0', 0);
	n = in.Read(buf, 1, sizeof(buf), in.self);
	CU_ASSERT(n == 12);
	buf[n] = '\0';
	CU_ASSERT_STRING_EQUAL(buf, "012345678901");

	in = Qiniu_SeqReader(&seq, 13, 10, '0', 1);
	n = in.Read(buf, 1, sizeof(buf), in.self);
	CU_ASSERT(n == 13);
	buf[n] = '\0';
	CU_ASSERT_STRING_EQUAL(buf, "1234567890123");

	in = Qiniu_SeqReader(&seq, 13, 10, '0', 1);
	h = Qiniu_Crc32Writer(&crc32, 0);
	in2 = Qiniu_TeeReader(&tee, in, h);
	n = in2.Read(buf, 1, sizeof(buf), in2.self);
	CU_ASSERT(n == 13);
	printf("crc32: %x\n", (int)crc32.val);
	CU_ASSERT(crc32.val == 0x74e38c01);

	in3 = Qiniu_SeqReaderAt(&seq, 1024, 10, '0', 0);
	in = Qiniu_SectionReader(&sect, in3, 1, 13);
	n = in.Read(buf, 1, sizeof(buf), in.self);
	CU_ASSERT(n == 13);
	buf[n] = '\0';
	CU_ASSERT_STRING_EQUAL(buf, "1234567890123");

	in = Qiniu_SectionReader(&sect, in3, 1, 13);
	crc32.val = 0;
	err = Qiniu_Copy(h, in, NULL, 4, NULL);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(crc32.val == 0x74e38c01);

	in = Qiniu_SectionReader(&sect, in3, 1, 13);
	crc32.val = 0;
	err = Qiniu_Copy(h, in, NULL, 16, NULL);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(crc32.val == 0x74e38c01);
}

