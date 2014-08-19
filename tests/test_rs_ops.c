#include "../qiniu/rs.h"
#include "test.h"
#include <stdio.h>
#include <stdlib.h>

static const char* copyNames[3] = { "testa.tmp", "testb.tmp", "testc.tmp" };
static const char* moveNames[3] = { "testa.mov.tmp", "testb.mov.tmp", "testc.mov.tmp" };
static const char bucket[] = "csdk";
static const char key[] = "key";
static const char domain[] = "csdk.qiniudn.com";

void debug(Qiniu_Client* client, Qiniu_Error err)
{
	printf("\nerror code: %d, message: %s\n", err.code, err.message);
	printf("respose header:\n%s", Qiniu_Buffer_CStr(&client->respHeader));
	printf("respose body:\n%s", Qiniu_Buffer_CStr(&client->b));
}

static void batchCopy(Qiniu_Client* client,
		Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchCopy(client, rets, entryPairs, entryCount);
	int curr = 0;

	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		}
		printf("\n%s\n", Qiniu_Buffer_CStr(&client->respHeader));
		CU_ASSERT(rets[curr].code == 200);
		curr++;
	}
	free(rets);

	if (err.code != 200) {
		debug(client, err);
		CU_ASSERT(err.code == 200);
		return;
	}
}

static void batchMove(Qiniu_Client* client,
		Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchMove(client, rets, entryPairs, entryCount);
	int curr = 0;

	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		}
		printf("\n%s\n", Qiniu_Buffer_CStr(&client->respHeader));
		CU_ASSERT(rets[curr].code == 200);
		curr++;
	}
	free(rets);

	if (err.code != 200) {
		debug(client, err);
		CU_ASSERT(err.code == 200);
		return;
	}
}

void batchStat(Qiniu_Client* client,
		Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchStatRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchStatRet));
	Qiniu_Error err = Qiniu_RS_BatchStat(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		} else {
			printf("hash: %s\n", rets[curr].data.hash);
			printf("mimeType: %s\n", rets[curr].data.mimeType);
			printf("fsize: %lld\n", rets[curr].data.fsize);
			printf("putTime: %lld\n", rets[curr].data.putTime);
		}
		printf("\n%s\n", Qiniu_Buffer_CStr(&client->respHeader));
		CU_ASSERT(rets[curr].code == 200);
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		CU_ASSERT(err.code == 200);
		return;
	}
}

void batchDelete(Qiniu_Client* client,
		Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchDelete(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		}
		printf("\n%s\n", Qiniu_Buffer_CStr(&client->respHeader));
		CU_ASSERT(rets[curr].code == 200);
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		CU_ASSERT(err.code == 200);
		return;
	}
}

void testRsBatchOps()
{
	Qiniu_Client client;
	Qiniu_RS_EntryPath entries[3];
	Qiniu_RS_EntryPathPair entryPairs[3];
	int i;

	Qiniu_Client_InitMacAuth(&client, 1024, NULL);

	for (i = 0; i < 3; i++) {
		entryPairs[i].src.bucket = bucket;
		entryPairs[i].dest.bucket = bucket;
		entryPairs[i].src.key = key;
		entryPairs[i].dest.key = copyNames[i];
	}
	batchCopy(&client, entryPairs, 3);

	for (i = 0; i < 3; i++) {
		entryPairs[i].src.key = copyNames[i];
		entryPairs[i].dest.key = moveNames[i];
	}
	batchMove(&client, entryPairs, 3);

	for (i = 0; i < 3; i++) {
		entries[i].bucket = bucket;
		entries[i].key = moveNames[i];
	}
	batchStat(&client, entries, 3);
	batchDelete(&client, entries, 3);

	Qiniu_Client_Cleanup(&client);
}
