#include <stdio.h>
#include <stdlib.h>
#include "../qiniu/http.h"
#include "../qiniu/cdn.h"

int main(int argc, char * argv[])
{
	Qiniu_Error err;
	Qiniu_Client cli;
	Qiniu_Mac mac;
    Qiniu_Cdn_RefreshRet ret;

	Qiniu_Global_Init(0);
	Qiniu_Servend_Init(0);
	Qiniu_MacAuth_Init();

	mac.accessKey = argv[1];
	mac.secretKey = argv[2];

    char* urls[2] = {"http://a.com/1.html","http://b.com/2.html"};

	Qiniu_Client_InitMacAuth(&cli, 1024, &mac);

    err = Qiniu_Cdn_RefreshUrls(&cli,&ret,urls,2);

	printf("code:%d\n msg:%s\n", err.code, err.message);

	if (err.code == 200) {
		printf("-----------------------------------------\n");
		printf("         code : %d\n", ret.code);
		printf("        error : %s\n", ret.error);
		printf("    requestId : %s\n", ret.requestId);
		//printf("  invalidUrls : %s\n", ret.invalidUrls);
		//printf("  invalidDirs : %s\n", ret.invalidDirs);
		printf("  urlQuotaDay : %d\n", ret.urlQuotaDay);
		printf("urlSurplusDay : %d\n", ret.urlSurplusDay);
		printf("  dirQuotaDay : %d\n", ret.dirQuotaDay);
		printf("dirSurplusDay : %d\n", ret.dirSurplusDay);
		printf("-----------------------------------------\n");
	}

	Qiniu_Client_Cleanup(&cli);
	//Qiniu_MacAuth_Cleanup();
	Qiniu_Servend_Cleanup();
	Qiniu_Global_Cleanup();

	return 0;
} // main
