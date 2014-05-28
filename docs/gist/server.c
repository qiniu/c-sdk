#include "../../qiniu/rs.h"

/* @gist debug */
void debug(Qiniu_Client* client, Qiniu_Error err)
{
	printf("\nerror code: %d, message: %s\n", err.code, err.message);
	printf("respose header:\n%s", Qiniu_Buffer_CStr(&client->respHeader));
	printf("respose body:\n%s", Qiniu_Buffer_CStr(&client->b));
}
/* @endgist */

/* @gist stat */
void stat(Qiniu_Client* client, const char* bucket, const char* key)
{
	Qiniu_RS_StatRet ret;
	Qiniu_Error err = Qiniu_RS_Stat(client, &ret, bucket, key);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("hash: %s, fsize: %lld, mimeType: %s\n", ret.hash, ret.fsize, ret.mimeType);
}
/* @endgist */

/* @gist delete */
void delete(Qiniu_Client* client, const char* bucket, const char* key)
{
	Qiniu_Error err = Qiniu_RS_Delete(client, bucket, key);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("%s:%s delete OK.\n", bucket, key);
}
/* @endgist */

/* @gist copy */
void copy(Qiniu_Client* client, 
	const char* bucketSrc, const char* keySrc, 
	const char* bucketDest, const char* keyDest)
{
	Qiniu_Error err = Qiniu_RS_Copy(client, bucketSrc, keySrc, bucketDest, keyDest);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("Copy %s:%s -> %s:%s OK.\n", bucketSrc, keySrc, bucketDest, keyDest);
}
/* @endgist */

/* @gist move */
void move(Qiniu_Client* client, 
	const char* bucketSrc, const char* keySrc, 
	const char* bucketDest, const char* keyDest)
{
	Qiniu_Error err = Qiniu_RS_Move(client, bucketSrc, keySrc, bucketDest, keyDest);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("Move %s:%s -> %s:%s OK.\n", bucketSrc, keySrc, bucketDest, keyDest);
}
/* @endgist */

/* @gist batchStat */
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
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
/* @endgist */

/* @gist batchDelete */
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
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
/* @endgist */

/* @gist batchMove */
void batchMove(Qiniu_Client* client, 
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
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
/* @endgist */

/* @gist batchCopy */
void batchCopy(Qiniu_Client* client, 
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
		curr++;
	}
	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
/* @endgist */

/* @gist uptoken */
char* uptoken(Qiniu_Client* client, const char* bucket)
{
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	return Qiniu_RS_PutPolicy_Token(&putPolicy, NULL);
}
/* @endgist */

/* @gist downloadUrl */
char* downloadUrl(Qiniu_Client* client, const char* domain, const char* key)
{
	char* url;
	char* baseUrl;
	Qiniu_RS_GetPolicy getPolicy;

	Qiniu_Zero(getPolicy);
	baseUrl = Qiniu_RS_MakeBaseUrl(domain, key); // baseUrl也可以自己拼接："http://"+domain+"/"+urlescape(key)
	url = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, baseUrl, NULL);

	Qiniu_Free(baseUrl);
	return url;                                  // When url is no longer being used, free it by Qiniu_Free.
}
/* @endgist */

int main()
{
	/* @gist init */
	Qiniu_Client client;

	QINIU_ACCESS_KEY = "<Please apply your access key>";
	QINIU_SECRET_KEY = "<Dont send your secret key to anyone>";

	Qiniu_Servend_Init(-1);                        /* 全局初始化函数，整个进程只需要调用一次 */
	Qiniu_Client_InitMacAuth(&client, 1024, NULL); /* HTTP客户端初始化。HTTP客户端是线程不安全的，不要在多个线程间共用 */
	/* @endgist */

	stat(&client, "bucket", "key");
	delete(&client, "bucket", "key");
	copy(&client, "bucket1", "key1", "bucket2", "key2");
	move(&client, "bucket1", "key1", "bucket2", "key2");

	/* batchstat/batchdelete */
	Qiniu_RS_EntryPath entries[2];
	entries[0].bucket = "bucket1";
	entries[0].key = "key1";
	entries[1].bucket = "bucket2";
	entries[1].key = "key2";

	batchStat(&client, entries, 2);
	batchDelete(&client, entries, 2);

	/* batchcopy/batchmove */
	Qiniu_RS_EntryPathPair entryPairs[2];
	entryPairs[0].src.bucket = "bucket1";
	entryPairs[0].src.key = "key1";
	entryPairs[0].dest.bucket = "bucket2";
	entryPairs[0].dest.key = "key2";

	entryPairs[1].src.bucket = "bucket3";
	entryPairs[1].src.key = "key3";
	entryPairs[1].dest.bucket = "bucket4";
	entryPairs[1].dest.key = "key4";

	batchCopy(&client, entryPairs, 2);
	batchMove(&client, entryPairs, 2);

	/* @gist init */
	Qiniu_Client_Cleanup(&client);                 /* 每个HTTP客户端使用完后释放 */
	Qiniu_Servend_Cleanup();                       /* 全局清理函数，只需要在进程退出时调用一次 */
	/* @endgist */
}
