#include "../qiniu/rs.h"
#include "../qiniu/http.h"
#include "../qiniu/resumable_io.h"

static int notifyCallback(void *recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet *ret)
{
	printf("blkIdx = %d, blkSize = %d, ctx = %s, crc32 = %d, host = %s\n", blkIdx, blkSize, ret->ctx, ret->crc32, ret->host);
	return QINIU_RIO_NOTIFY_OK;
}

static void resumableUploadWithKey(Qiniu_Mac *mac, const char *bucket, const char *key, const char *localFile)
{
	Qiniu_Rio_Settings rioSettings;
	Qiniu_Zero(rioSettings);
	rioSettings.chunkSize = 4 * 1024 * 1024;
	Qiniu_Rio_SetSettings(&rioSettings);

	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	char *upToken = Qiniu_RS_PutPolicy_Token(&putPolicy, mac);

	Qiniu_Client client;
	Qiniu_Global_Init(-1);
	Qiniu_Client_InitNoAuth(&client, 1024);

	Qiniu_Error error;
	Qiniu_Rio_PutRet putRet;
	Qiniu_Zero(putRet);
	Qiniu_Rio_PutExtra putExtra;
	Qiniu_Zero(putExtra);
	Qiniu_Recorder recorder;
	Qiniu_Zero(recorder);
	error = Qiniu_FileSystem_Recorder_New("/tmp", &recorder);
	if (error.code != Qiniu_OK.code)
	{
		fprintf(stderr, "code: %d\n", error.code);
		fprintf(stderr, "message: %s\n", error.message);
		goto Exit;
	}

	putExtra.notify = notifyCallback;
	putExtra.recorder = &recorder;
	error = Qiniu_Rio_PutFile(&client, &putRet, upToken, key, localFile, &putExtra);

	if (error.code != Qiniu_OK.code)
	{
		fprintf(stderr, "code: %d\n", error.code);
		fprintf(stderr, "message: %s\n", error.message);
	}
	else
	{
		printf("Key: %s\n", putRet.key);
		printf("Hash: %s\n", putRet.hash);
	}
Exit:
	recorder.free(&recorder);
	Qiniu_Client_Cleanup(&client);
	Qiniu_Global_Cleanup();
	Qiniu_Free(upToken);
}

int main()
{
	char *accessKey = getenv("QINIU_ACCESS_KEY");
	char *secretKey = getenv("QINIU_SECRET_KEY");
	char *bucket = getenv("QINIU_TEST_BUCKET");
	char *key = "testkey";
	char *localFile = __FILE__;

	Qiniu_Mac mac;
	mac.accessKey = accessKey;
	mac.secretKey = secretKey;

	resumableUploadWithKey(&mac, bucket, key, localFile);

	return 0;
}
