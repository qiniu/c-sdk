#include "../../qiniu/rs.h"

/* @gist debug */
void debug(Qiniu_Client* client, Qiniu_Error err)
{
	printf("error code: %d, message: %s\n", err.code, err.message);
	printf("respose header:\n%s", Qiniu_Buffer_CStr(&client->respHeader));
	printf("respose body:\n%s", Qiniu_Buffer_CStr(&client->b));
}
/* @endgist */

/* @gist stat */
void stat(Qiniu_Client* client, const char* bucket)
{
	Qiniu_RS_StatRet ret;
	Qiniu_Error err = Qiniu_RS_Stat(client, &ret, bucket, "key");
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("hash: %s, fsize: %lld, mimeType: %s\n", ret.hash, ret.fsize, ret.mimeType);
}
/* @endgist */

int main()
{
	/* @gist init */
	Qiniu_Client client;

	QINIU_ACCESS_KEY = "<Please apply your access key>";
	QINIU_SECRET_KEY = "<Dont send your secret key to anyone>";

	Qiniu_Global_Init(-1);                  /* 全局初始化函数，整个进程只需要调用一次 */
	Qiniu_Client_Init(&client, 1024);       /* HTTP客户端初始化。HTTP客户端实例是线程不安全的，每个线程独立使用，互不相干 */
	/* @endgist */

	stat(&client, "a");

	/* @gist init */
	Qiniu_Client_Cleanup(&client);           /* 每个HTTP客户端使用完后释放 */
	Qiniu_Global_Cleanup();                 /* 全局清理函数，只需要在进程退出时调用一次 */
	/* @endgist */
}
