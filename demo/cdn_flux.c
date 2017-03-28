#include <stdio.h>
#include <stdlib.h>
#include "../qiniu/http.h"
#include "../qiniu/cdn.h"

int main(int argc, char * argv[])
{
	Qiniu_Error err;
	Qiniu_Client cli;
	Qiniu_Mac mac;
	Qiniu_Cdn_FluxRet ret;

	Qiniu_Global_Init(0);
	Qiniu_Servend_Init(0);
	Qiniu_MacAuth_Init();

	mac.accessKey = argv[1];
	mac.secretKey = argv[2];

	char* startDate = "2017-03-28";
	char* endDate = "2017-03-28";
	char* granularity = "day";
	char* domains[2] = { "a.com","b.com" };

	Qiniu_Client_InitMacAuth(&cli, 1024, &mac);

	err = Qiniu_Cdn_GetFluxData(&cli, &ret, startDate, endDate, granularity, domains, 2);

	printf("code:%d\n msg:%s\n", err.code, err.message);

	printf("---------------------------------------------------------------------\n");
	printf(" code : %d\n", ret.code);
	printf("error : %s\n", ret.error);
	printf("  num : %d\n", ret.num);
	for (int i = 0; i < ret.num; ++i) {
		if (ret.data_a[i].hasValue != FALSE) {
			printf("\ndomain : %s\n", ret.data_a[i].domain);
			printf("#\t\ttime\t\tchina\t\toversea\n");
			for (int j = 0; j < ret.data_a[i].count; ++j) {
				printf("%d\t%s\t%d\t%d\n",
					j + 1,
					ret.data_a[i].item_a[j].time,
					ret.data_a[i].item_a[j].val_china,
					ret.data_a[i].item_a[j].val_oversea);
			}
		}
	}
	printf("---------------------------------------------------------------------\n");

	Qiniu_Free_CdnFluxRet(&ret);

	Qiniu_Client_Cleanup(&cli);
	//Qiniu_MacAuth_Cleanup();
	Qiniu_Servend_Cleanup();
	Qiniu_Global_Cleanup();

	return 0;
} // main
