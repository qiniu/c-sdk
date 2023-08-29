#include <gtest/gtest.h>
#include "test.h"
#include "qiniu/base.h"
#include "qiniu/multipart_upload.h"
#include "qiniu/tm.h"
#include "qiniu/rs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#ifdef _WIN32
#include <windows.h>
#endif

static const char *putMemoryData_multipart(const char *bucket, const char *key, const char *mimeType, const char *memData, int dataLen, Qiniu_Mac *mac)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Multipart_PutExtra putExtra;
	Qiniu_MultipartUpload_Result putRet;
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_Zero(client);
	Qiniu_Zero(putPolicy);
	Qiniu_Zero(putRet);
	Qiniu_Zero(putExtra);
	putPolicy.scope = bucket;
	char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, mac);

	putExtra.mimeType = mimeType;
	putExtra.enableContentMd5 = 0;
	putExtra.partSize = (4 << 20);
	putExtra.tryTimes = 2;

	Qiniu_Client_InitMacAuth(&client, 1024, mac);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	// construct reader by memory data
	Qiniu_ReadBuf rbuff;
	Qiniu_ReaderAt reader = Qiniu_BufReaderAt(&rbuff, memData, (size_t)dataLen);
	err = Qiniu_Multipart_Put(&client, uptoken, key, reader, dataLen, &putExtra, &putRet);

	EXPECT_EQ(err.code, 200);
	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Log_Debug("hash: %s , key:%s", putRet.hash, putRet.key);

	Qiniu_Client_Cleanup(&client);
	Qiniu_Free(uptoken);
	return putRet.key;
}

static const char *putFile_multipart(const char *bucket, const char *key, const char *mimeType, const char *filePath, Qiniu_Int64 partSize, Qiniu_Mac *mac)
{
	Qiniu_Error err;
	Qiniu_Client client;
	Qiniu_Multipart_PutExtra putExtra;
	Qiniu_MultipartUpload_Result putRet;
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_Recorder recorder;
	Qiniu_Zero(client);
	Qiniu_Zero(putPolicy);
	Qiniu_Zero(putRet);
	Qiniu_Zero(putExtra);
	Qiniu_Zero(recorder);
	putPolicy.scope = bucket;
	char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, mac);

#ifdef _WIN32
	char tempDirPath[MAX_PATH], dirPathSuffix[20];
	GetTempPath(MAX_PATH, tempDirPath);
	snprintf(dirPathSuffix, 20, "%d\\", rand());
	strncat(tempDirPath, dirPathSuffix, MAX_PATH);
	EXPECT_TRUE(CreateDirectoryA(tempDirPath, NULL));
#else
	char tempDirPath[PATH_MAX];
	snprintf(tempDirPath, PATH_MAX, "/tmp/%d/", rand());
	EXPECT_EQ(mkdir(tempDirPath, 0700), 0);
#endif

	err = Qiniu_FileSystem_Recorder_New(tempDirPath, &recorder);
	EXPECT_EQ(err.code, 200);
	putExtra.recorder = &recorder;
	putExtra.mimeType = mimeType;
	putExtra.enableContentMd5 = 1;
	putExtra.partSize = partSize;
	putExtra.tryTimes = 2;

	Qiniu_Client_InitMacAuth(&client, 1024, mac);
	Qiniu_Client_SetTimeout(&client, 120000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	err = Qiniu_Multipart_PutFile(&client, uptoken, key, filePath, &putExtra, &putRet);

	EXPECT_EQ(err.code, 200);
	Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client.respHeader));
	Qiniu_Log_Debug("hash: %s , key:%s", putRet.hash, putRet.key);

	recorder.free(&recorder);
	Qiniu_Client_Cleanup(&client);
	Qiniu_Free(uptoken);
	return putRet.key;
}

TEST(IntegrationTest, TestMultipartUpload_smallfile)
{
	Qiniu_Client client;
	Qiniu_Zero(client);

	Qiniu_Error err;
	Qiniu_Mac mac = {QINIU_ACCESS_KEY, QINIU_SECRET_KEY}; // set by env "source test-env.sh"
	Qiniu_Client_InitMacAuth(&client, 1024, &mac);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	char smallFileKey[100];
	Qiniu_snprintf(smallFileKey, 100, "smallkey_%d", rand());

	const char *keys[] = {
		smallFileKey, // normal keyname
		"",			  // empty string keyname
		NULL};		  // no keyname, determined by server(eg:hash as keyname)
	for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
	{
		const char *inputKey = keys[i];

		// step1: delete  file if exist
		if (inputKey != NULL)
		{
			Qiniu_RS_Delete(&client, Test_bucket, inputKey);
		}
		// step2: upload file
		const char *returnKey = putFile_multipart(Test_bucket, inputKey, "txt", __FILE__, (4 << 20), &mac); // upload current file

		// step3: stat file
		Qiniu_RS_StatRet statResult;
		err = Qiniu_RS_Stat(&client, &statResult, Test_bucket, returnKey);
		EXPECT_EQ(err.code, 200);
		EXPECT_STREQ(statResult.mimeType, "txt");

		// step4: delete file
		err = Qiniu_RS_Delete(&client, Test_bucket, returnKey);
		EXPECT_EQ(err.code, 200);
	}

	Qiniu_Client_Cleanup(&client);
}

TEST(IntegrationTest, TestMultipartUpload_largefile)
{
	Qiniu_Int64 partSize[] = {2 << 20, 4 << 20};
	for (int i = 0; i < sizeof(partSize) / sizeof(partSize[0]); i++)
	{
		Qiniu_Client client;
		Qiniu_Zero(client);

		Qiniu_Error err;
		Qiniu_Mac mac = {QINIU_ACCESS_KEY, QINIU_SECRET_KEY};
		Qiniu_Client_InitMacAuth(&client, 1024, &mac);
		Qiniu_Client_SetTimeout(&client, 5000);
		Qiniu_Client_SetConnectTimeout(&client, 3000);
		Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

		char inputKey[100];
		Qiniu_snprintf(inputKey, 100, "largefile_%d", rand());

		// step1: delete  file if exist
		Qiniu_RS_Delete(&client, Test_bucket, inputKey);

		// step2: upload file
		char filePath[PATH_MAX] = {0};
		strcpy(filePath, __SOURCE_DIR__);
		strcat(filePath, "/gtests/resources/test5m.mp3");
		const char *returnKey = putFile_multipart(Test_bucket, inputKey, "mp3", (const char *)filePath, partSize[i], &mac);

		// step3: stat file
		Qiniu_RS_StatRet statResult;
		err = Qiniu_RS_Stat(&client, &statResult, Test_bucket, returnKey);
		EXPECT_EQ(err.code, 200);
		EXPECT_STREQ(statResult.mimeType, "mp3");
		EXPECT_EQ(statResult.fsize, 5097014);

		// step4: delete file
		err = Qiniu_RS_Delete(&client, Test_bucket, returnKey);
		EXPECT_EQ(err.code, 200);

		Qiniu_Client_Cleanup(&client);
	}
}

TEST(IntegrationTest, TestMultipartUpload_emptyfile)
{
	Qiniu_Client client;
	Qiniu_Zero(client);

	Qiniu_Error err;
	Qiniu_Mac mac = {QINIU_ACCESS_KEY, QINIU_SECRET_KEY};
	Qiniu_Client_InitMacAuth(&client, 1024, &mac);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	char inputKey[100];
	Qiniu_snprintf(inputKey, 100, "emptyfile_%d", rand());

	// step1: delete  file if exist
	Qiniu_RS_Delete(&client, Test_bucket, inputKey);

	// step2: upload file
	char filePath[PATH_MAX] = {0};
	strcpy(filePath, __SOURCE_DIR__);
	strcat(filePath, "/gtests/resources/test_emptyfile.txt");
	const char *returnKey = putFile_multipart(Test_bucket, inputKey, "txt", filePath, (4 << 20), &mac);

	// step3: stat file
	Qiniu_RS_StatRet statResult;
	err = Qiniu_RS_Stat(&client, &statResult, Test_bucket, returnKey);
	EXPECT_EQ(err.code, 200);
	EXPECT_EQ(statResult.fsize, 0);
	EXPECT_STREQ(statResult.mimeType, "txt");

	// step4: delete file
	err = Qiniu_RS_Delete(&client, Test_bucket, returnKey);
	EXPECT_EQ(err.code, 200);

	Qiniu_Client_Cleanup(&client);
}

TEST(IntegrationTest, TestMultipartUpload_inMemoryData)
{
	Qiniu_Client client;
	Qiniu_Zero(client);

	Qiniu_Error err;
	Qiniu_Mac mac = {QINIU_ACCESS_KEY, QINIU_SECRET_KEY};
	Qiniu_Client_InitMacAuth(&client, 1024, &mac);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);
	Qiniu_Client_EnableAutoQuery(&client, Qiniu_True);

	char inputKey[100];
	Qiniu_snprintf(inputKey, 100, "memoryDataKey_%d", rand());

	// step1: delete  file if exist
	Qiniu_RS_Delete(&client, Test_bucket, inputKey);

	// step2: upload memory data
	const char memData[] = "test multipart upload with memory data";
	const char *returnKey = putMemoryData_multipart(Test_bucket, inputKey, NULL, memData, sizeof(memData), &mac);

	// step3: stat file
	Qiniu_RS_StatRet statResult;
	err = Qiniu_RS_Stat(&client, &statResult, Test_bucket, returnKey);
	EXPECT_EQ(err.code, 200);
	EXPECT_EQ(statResult.fsize, sizeof(memData));

	// step4: delete file
	err = Qiniu_RS_Delete(&client, Test_bucket, returnKey);
	EXPECT_EQ(err.code, 200);

	Qiniu_Client_Cleanup(&client);
}
