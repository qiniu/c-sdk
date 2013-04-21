#include "../../qiniu/io.h"

int main()
{
	/* @gist init */
	Qiniu_Client client;

	Qiniu_Global_Init(-1);                  /* 全局初始化函数，整个进程只需要调用一次 */
	Qiniu_Client_InitNoAuth(&client, 1024); /* HTTP客户端初始化。HTTP客户端实例是线程不安全的，每个线程独立使用，互不相干 */
	/* @endgist */

	/* @gist init */
	Qiniu_Client_Cleanup(&client);           /* 每个HTTP客户端使用完后释放 */
	Qiniu_Global_Cleanup();                 /* 全局清理函数，只需要在进程退出时调用一次 */
	/* @endgist */
}

