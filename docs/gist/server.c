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

	stat(&client, "a");

	/* @gist init */
	Qiniu_Client_Cleanup(&client);                 /* 每个HTTP客户端使用完后释放 */
	Qiniu_Servend_Cleanup();                       /* 全局清理函数，只需要在进程退出时调用一次 */
	/* @endgist */
}
