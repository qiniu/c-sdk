/*
 ============================================================================
 Name        : test_recorder.c
 Author      : Qiniu.com
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : Qiniu C SDK Unit Test
 ============================================================================
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../qiniu/recorder.h"

void testFileSystemRecorder_readWriteEntries(void)
{
	Qiniu_Error err;
	Qiniu_Recorder recorder;
	Qiniu_Record_Medium medium;
	size_t haveWritten, haveRead;
	Qiniu_Bool hasNext;
	const char *KEY = "aaaaa";
	char buf[20];
	const size_t BUF_LEN = 20;

	Qiniu_FileSystem_Recorder_New("/tmp", &recorder);
	err = recorder.open(&recorder, KEY, "wb", &medium);
	CU_ASSERT(err.code == 200);

	const char *DATA_1 = "abcdefghijkl";
	err = medium.writeEntry(&medium, DATA_1, &haveWritten);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(haveWritten == strlen(DATA_1));

	const char *DATA_2 = "ABCDEFGHIJKL";
	err = medium.writeEntry(&medium, DATA_2, &haveWritten);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(haveWritten == strlen(DATA_2));

	err = medium.close(&medium);
	CU_ASSERT(err.code == 200);

	err = recorder.open(&recorder, "aaaaa", "rb", &medium);
	CU_ASSERT(err.code == 200);

	err = medium.hasNextEntry(&medium, &hasNext);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(hasNext == Qiniu_True);

	memset(buf, 0, BUF_LEN);
	err = medium.readEntry(&medium, buf, BUF_LEN, &haveRead);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(haveRead == strlen(DATA_1));
	CU_ASSERT(strncmp(buf, DATA_1, BUF_LEN) == 0);

	err = medium.hasNextEntry(&medium, &hasNext);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(hasNext == Qiniu_True);

	memset(buf, 0, BUF_LEN);
	err = medium.readEntry(&medium, buf, BUF_LEN, &haveRead);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(haveRead == strlen(DATA_2));
	CU_ASSERT(strncmp(buf, DATA_2, BUF_LEN) == 0);

	err = medium.hasNextEntry(&medium, &hasNext);
	CU_ASSERT(err.code == 200);
	CU_ASSERT(hasNext == Qiniu_False);

	err = recorder.remove(&recorder, KEY);
	CU_ASSERT(err.code == 200);

	recorder.free(&recorder);
}
