#include "../../qiniu/io.h"
#include "../../qiniu/resumable_io.h"

const char bucket[] = "a";

/* @gist debug */
void debug(Qiniu_Client* client, Qiniu_Error err)
{
	printf("error code: %d, message: %s\n", err.code, err.message);
	printf("respose header:\n%s", Qiniu_Buffer_CStr(&client->respHeader));
	printf("respose body:\n%s", Qiniu_Buffer_CStr(&client->b));
}
/* @endgist */

/* @gist upload */
char* upload(Qiniu_Client* client, char* uptoken, const char* key, const char* localFile)
{
	Qiniu_Error err;
	Qiniu_Io_PutRet putRet;
	err = Qiniu_Io_PutFile(client, &putRet, uptoken, key, localFile, NULL);
	if (err.code != 200) {
		debug(client, err);
		return NULL;
	}
	return strdup(putRet.hash); /* 注意需要后续使用的变量要复制出来 */
}
/* @endgist */

/* @gist simple-upload */
int simple_upload(Qiniu_Client* client, char* uptoken, const char* key, const char* localFile)
{
	Qiniu_Error err;
	err = Qiniu_Io_PutFile(client, NULL, uptoken, key, localFile, NULL);
	return err.code;
}
/* @endgist */

/* @gist resumable-upload */
int resumable_upload(Qiniu_Client* client, char* uptoken, const char* key, const char* localFile)
{
	Qiniu_Error err;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Zero(extra);
	extra.bucket = bucket;
	err = Qiniu_Rio_PutFile(client, NULL, uptoken, key, localFile, &extra);
	return err.code;
}
/* @endgist */

int main()
{
	/* @gist init */
	Qiniu_Client client;

	Qiniu_Global_Init(-1);                  /* 全局初始化函数，整个进程只需要调用一次 */
	Qiniu_Client_InitNoAuth(&client, 1024); /* HTTP客户端初始化。HTTP客户端是线程不安全的，不要在多个线程间共用 */
	/* @endgist */

	/* @gist init */
	Qiniu_Client_Cleanup(&client);          /* 每个HTTP客户端使用完后释放 */
	Qiniu_Global_Cleanup();                 /* 全局清理函数，只需要在进程退出时调用一次 */
	/* @endgist */
}

