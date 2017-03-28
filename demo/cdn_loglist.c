#include <stdio.h>
#include <stdlib.h>
#include "../qiniu/http.h"
#include "../qiniu/cdn.h"

int main(int argc, char * argv[])
{
	Qiniu_Error err;
	Qiniu_Client cli;
	Qiniu_Mac mac;
	Qiniu_Cdn_LogListRet ret;

	Qiniu_Global_Init(0);
	Qiniu_Servend_Init(0);
	Qiniu_MacAuth_Init();

	mac.accessKey = argv[1];
	mac.secretKey = argv[2];

	char* day = "2017-03-28";
	char* domains[2] = { "a.com","b.com" };

	Qiniu_Client_InitMacAuth(&cli, 1024, &mac);

	err = Qiniu_Cdn_GetLogList(&cli, &ret, day, domains, 2);

	printf("code:%d\n msg:%s\n", err.code, err.message);

	printf("---------------------------------------------------\n");
	printf(" code : %d\n", ret.code);
	printf("error : %s\n", ret.error);
	printf("  num : %d\n", ret.num);
	for (int i = 0; i < ret.num; ++i) {
		if (ret.data_a[i].hasValue) {
			printf("\n");
			printf("  index : %d/%d\n", i + 1, ret.num);
			printf(" domain : %s\n\n", ret.data_a[i].domain);
			for (int j = 0; j < ret.data_a[i].count; ++j) {
				printf("   part : %d/%d\n", j + 1, ret.data_a[i].count);
				printf("   name : %s\n", ret.data_a[i].item_a[j].name);
				printf("   size : %d\n", ret.data_a[i].item_a[j].size);
				printf("  mtime : %d\n", ret.data_a[i].item_a[j].mtime);
				printf("    url : %s\n", ret.data_a[i].item_a[j].url);
				printf("\n");
			}
			printf("\n");
		}
	}
	printf("---------------------------------------------------\n");

	Qiniu_Free_CdnLogListRet(&ret);

	Qiniu_Client_Cleanup(&cli);
	//Qiniu_MacAuth_Cleanup();
	Qiniu_Servend_Cleanup();
	Qiniu_Global_Cleanup();

	return 0;
} // main
