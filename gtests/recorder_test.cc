#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "test.h"
#include "qiniu/recorder.h"

TEST(UnitTest, TestFileSystemRecorder_readWriteEntries)
{
	Qiniu_Error err;
	Qiniu_Recorder recorder;
	Qiniu_Record_Medium medium;
	size_t haveWritten, haveRead;
	Qiniu_Bool hasNext;
	const char *KEY = "aaaaa";
	char buf[20];
	const size_t BUF_LEN = 20;

	Qiniu_FileSystem_Recorder_New(TEMP_DIR, &recorder);
	err = recorder.open(&recorder, KEY, "wb", &medium);
	EXPECT_EQ(err.code, Qiniu_OK.code);

	const char *DATA_1 = "abcdefghijkl";
	err = medium.writeEntry(&medium, DATA_1, &haveWritten);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_EQ(haveWritten, strlen(DATA_1));

	const char *DATA_2 = "ABCDEFGHIJKL";
	err = medium.writeEntry(&medium, DATA_2, &haveWritten);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_EQ(haveWritten, strlen(DATA_2));

	err = medium.close(&medium);
	EXPECT_EQ(err.code, Qiniu_OK.code);

	err = recorder.open(&recorder, "aaaaa", "rb", &medium);
	EXPECT_EQ(err.code, Qiniu_OK.code);

	err = medium.hasNextEntry(&medium, &hasNext);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_TRUE(hasNext);

	memset(buf, 0, BUF_LEN);
	err = medium.readEntry(&medium, buf, BUF_LEN, &haveRead);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_EQ(haveRead, strlen(DATA_1));
	EXPECT_EQ(strncmp(buf, DATA_1, BUF_LEN), 0);

	err = medium.hasNextEntry(&medium, &hasNext);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_TRUE(hasNext);

	memset(buf, 0, BUF_LEN);
	err = medium.readEntry(&medium, buf, BUF_LEN, &haveRead);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_EQ(haveRead, strlen(DATA_2));
	EXPECT_EQ(strncmp(buf, DATA_2, BUF_LEN), 0);

	err = medium.hasNextEntry(&medium, &hasNext);
	EXPECT_EQ(err.code, Qiniu_OK.code);
	EXPECT_FALSE(hasNext);

	err = recorder.remove(&recorder, KEY);
	EXPECT_EQ(err.code, Qiniu_OK.code);

	recorder.free(&recorder);
}
