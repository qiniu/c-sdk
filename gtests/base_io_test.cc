#include <gtest/gtest.h>
#include "test.h"

static const Qiniu_Int64 fsize = 4 * 1024 * 1024 + 2;

TEST(UnitTest, TestEqual)
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
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_EQ(ncopy, fsize);
	EXPECT_TRUE(Qiniu_Is(&eq));
}

TEST(UnitTest, TestFileIO)
{
	char buf[24];
	size_t len = 20;
	size_t n;
	Qiniu_File *fp;

	Qiniu_Error err = Qiniu_File_Open(&fp, __FILE__);
	EXPECT_EQ(err.code, Qiniu_OK.code);

	buf[len] = '\0';

	n = Qiniu_File_ReadAt(fp, buf, len, 0);
	Qiniu_Log_Debug("%s, %d, %d, %d", buf, n, len, errno);
	EXPECT_EQ(n, len);

	n = Qiniu_File_ReadAt(fp, buf, len, 2);
	Qiniu_Log_Debug("%s, %d, %d, %d", buf, n, len, errno);
	EXPECT_EQ(n, len);

	Qiniu_File_Close(fp);
}

TEST(UnitTest, TestBaseIO)
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
	EXPECT_EQ(n, 12);
	buf[n] = '\0';
	EXPECT_STREQ(buf, "012345678901");

	in = Qiniu_SeqReader(&seq, 13, 10, '0', 1);
	n = in.Read(buf, 1, sizeof(buf), in.self);
	EXPECT_EQ(n, 13);
	buf[n] = '\0';
	EXPECT_STREQ(buf, "1234567890123");

	in = Qiniu_SeqReader(&seq, 13, 10, '0', 1);
	h = Qiniu_Crc32Writer(&crc32, 0);
	in2 = Qiniu_TeeReader(&tee, in, h);
	n = in2.Read(buf, 1, sizeof(buf), in2.self);
	EXPECT_EQ(n, 13);
	Qiniu_Log_Debug("crc32: %d", (int)crc32.val);
	EXPECT_EQ(crc32.val, 0x74e38c01);

	in3 = Qiniu_SeqReaderAt(&seq, 1024, 10, '0', 0);
	in = Qiniu_SectionReader(&sect, in3, 1, 13);
	n = in.Read(buf, 1, sizeof(buf), in.self);
	EXPECT_EQ(n, 13);
	buf[n] = '\0';
	EXPECT_STREQ(buf, "1234567890123");

	in = Qiniu_SectionReader(&sect, in3, 1, 13);
	crc32.val = 0;
	err = Qiniu_Copy(h, in, NULL, 4, NULL);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_EQ(crc32.val, 0x74e38c01);

	in = Qiniu_SectionReader(&sect, in3, 1, 13);
	crc32.val = 0;
	err = Qiniu_Copy(h, in, NULL, 16, NULL);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_EQ(crc32.val, 0x74e38c01);
}
