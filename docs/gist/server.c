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

/* @gist uptoken */
char* uptoken(Qiniu_Client* client, const char* bucket)
{
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	return Qiniu_RS_PutPolicy_Token(&putPolicy);
}
/* @endgist */

/* @gist dntoken */
char* dntoken(Qiniu_Client* client, const char* key)
{
	char* token;
	Qiniu_RS_GetPolicy getPolicy;
	Qiniu_Zero(getPolicy);
	getPolicy.scope = "*/*"; /* 错！！！下载授权切记不要授权范围过大，否则容易导致安全隐患 */
	getPolicy.scope = Qiniu_String_Concat2("*/", key); /* 正确！只授权这一个资源可以被访问 */
	token = Qiniu_RS_GetPolicy_Token(&getPolicy);
	free((void*)getPolicy.scope);
	return token;
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
