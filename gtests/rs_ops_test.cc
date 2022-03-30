#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "qiniu/rs.h"

static const char *copyNames[3] = {"testa.tmp", "testb.tmp", "testc.tmp"};
static const char *moveNames[3] = {"testa.mov.tmp", "testb.mov.tmp", "testc.mov.tmp"};
static const char key[] = "key";

static void debug(Qiniu_Client *client, Qiniu_Error err)
{
	Qiniu_Log_Debug("error code: %d, message: %s", err.code, err.message);
	Qiniu_Log_Debug("respose header:%s", Qiniu_Buffer_CStr(&client->respHeader));
	Qiniu_Log_Debug("respose body:%s", Qiniu_Buffer_CStr(&client->b));
}

static void batchCopy(Qiniu_Client *client,
		      Qiniu_RS_EntryPathPair *entryPairs, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet *rets = (Qiniu_RS_BatchItemRet *)calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchCopy(client, rets, entryPairs, entryCount);
	int curr = 0;

	while (curr < entryCount)
	{
		Qiniu_Log_Debug("code: %d", rets[curr].code);

		if (rets[curr].code != 200)
		{
			Qiniu_Log_Error("error: %s", rets[curr].error);
		}
		Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client->respHeader));
		EXPECT_EQ(rets[curr].code, 200);
		curr++;
	}
	free(rets);

	if (err.code != 200)
	{
		debug(client, err);
		FAIL();
		return;
	}
}

static void batchMove(Qiniu_Client *client,
		      Qiniu_RS_EntryPathPair *entryPairs, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet *rets = (Qiniu_RS_BatchItemRet *)calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchMove(client, rets, entryPairs, entryCount);
	int curr = 0;

	while (curr < entryCount)
	{
		Qiniu_Log_Debug("code: %d", rets[curr].code);

		if (rets[curr].code != 200)
		{
			Qiniu_Log_Error("error: %s", rets[curr].error);
		}
		Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client->respHeader));
		EXPECT_EQ(rets[curr].code, 200);
		curr++;
	}
	free(rets);

	if (err.code != 200)
	{
		debug(client, err);
		FAIL();
		return;
	}
}

static void batchStat(Qiniu_Client *client,
		      Qiniu_RS_EntryPath *entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchStatRet *rets = (Qiniu_RS_BatchStatRet *)calloc(entryCount, sizeof(Qiniu_RS_BatchStatRet));
	Qiniu_Error err = Qiniu_RS_BatchStat(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount)
	{
		Qiniu_Log_Debug("code: %d", rets[curr].code);

		if (rets[curr].code != 200)
		{
			Qiniu_Log_Error("error: %s", rets[curr].error);
		}
		else
		{
			Qiniu_Log_Debug("hash: %s", rets[curr].data.hash);
			Qiniu_Log_Debug("mimeType: %s", rets[curr].data.mimeType);
			Qiniu_Log_Debug("fsize: %D", rets[curr].data.fsize);
			Qiniu_Log_Debug("putTime: %D", rets[curr].data.putTime);
		}
		Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client->respHeader));
		EXPECT_EQ(rets[curr].code, 200);
		curr++;
	}

	free(rets);

	if (err.code != 200)
	{
		debug(client, err);
		FAIL();
		return;
	}
}

static void batchChangeType(Qiniu_Client *client,
		      Qiniu_RS_EntryChangeType *entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet *rets = (Qiniu_RS_BatchItemRet *)calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchChangeType(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount)
	{
		Qiniu_Log_Debug("code: %d", rets[curr].code);

		if (rets[curr].code != 200)
		{
			Qiniu_Log_Error("error: %s", rets[curr].error);
		}
		Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client->respHeader));
		EXPECT_EQ(rets[curr].code, 200);
		curr++;
	}
	free(rets);

	if (err.code != 200)
	{
		debug(client, err);
		FAIL();
		return;
	}
}

static void batchRestoreArchive(Qiniu_Client *client,
		      Qiniu_RS_EntryRestoreArchive *entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet *rets = (Qiniu_RS_BatchItemRet *)calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchRestoreArchive(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount)
	{
		Qiniu_Log_Debug("code: %d", rets[curr].code);

		if (rets[curr].code != 200)
		{
			Qiniu_Log_Error("error: %s", rets[curr].error);
		}
		Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client->respHeader));
		EXPECT_EQ(rets[curr].code, 200);
		curr++;
	}
	free(rets);

	if (err.code != 200)
	{
		debug(client, err);
		FAIL();
		return;
	}
}

static void batchDelete(Qiniu_Client *client,
			Qiniu_RS_EntryPath *entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet *rets = (Qiniu_RS_BatchItemRet *)calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchDelete(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount)
	{
		Qiniu_Log_Debug("code: %d", rets[curr].code);

		if (rets[curr].code != 200)
		{
			Qiniu_Log_Error("error: %s", rets[curr].error);
		}
		Qiniu_Log_Debug("%s", Qiniu_Buffer_CStr(&client->respHeader));
		EXPECT_EQ(rets[curr].code, 200);
		curr++;
	}

	free(rets);

	if (err.code != 200)
	{
		debug(client, err);
		FAIL();
		return;
	}
}

TEST(IntegrationTest, TestRsBatchOps)
{
	Qiniu_Client client;
	Qiniu_RS_EntryPath entries[3];
	Qiniu_RS_EntryPathPair entryPairs[3];
	Qiniu_RS_EntryChangeType entriesToChangeType[3];
	Qiniu_RS_EntryRestoreArchive entriesToRestore[3];
	int i;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);
	Qiniu_Client_SetTimeout(&client, 5000);
	Qiniu_Client_SetConnectTimeout(&client, 3000);

	for (i = 0; i < 3; i++)
	{
		entryPairs[i].src.bucket = Test_bucket;
		entryPairs[i].dest.bucket = Test_bucket;
		entryPairs[i].src.key = key;
		entryPairs[i].dest.key = copyNames[i];
	}
	batchCopy(&client, entryPairs, 3);

	for (i = 0; i < 3; i++)
	{
		entryPairs[i].src.key = copyNames[i];
		entryPairs[i].dest.key = moveNames[i];
	}
	batchMove(&client, entryPairs, 3);

	for (i = 0; i < 3; i++)
	{
		entries[i].bucket = Test_bucket;
		entries[i].key = moveNames[i];
	}
	batchStat(&client, entries, 3);

	for (i = 0; i < 3; i++) {
		entriesToChangeType[i].bucket = Test_bucket;
		entriesToChangeType[i].key = moveNames[i];
		entriesToChangeType[i].fileType = 0;
	}
	batchChangeType(&client, entriesToChangeType, 3);
	for (i = 0; i < 3; i++) {
		entriesToChangeType[i].bucket = Test_bucket;
		entriesToChangeType[i].key = moveNames[i];
		entriesToChangeType[i].fileType = 2;
	}
	batchChangeType(&client, entriesToChangeType, 3);

	for (i = 0; i < 3; i++) {
		entriesToRestore[i].bucket = Test_bucket;
		entriesToRestore[i].key = moveNames[i];
		entriesToRestore[i].deleteAfterDays = 1;
	}
	batchRestoreArchive(&client, entriesToRestore, 3);

	batchDelete(&client, entries, 3);

	Qiniu_Client_Cleanup(&client);
}
