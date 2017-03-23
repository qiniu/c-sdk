#include <stdio.h>
#include <stdlib.h>
#include "../qiniu/http.h"
#include "../qiniu/cdn.h"

int main(int argc, char * argv[])
{
	Qiniu_Error err;
	Qiniu_Client cli;
	Qiniu_Mac mac;
    Qiniu_Cdn_FluxBandwidthRet ret;

	Qiniu_Global_Init(0);
	Qiniu_Servend_Init(0);
	Qiniu_MacAuth_Init();

	mac.accessKey = argv[1];
	mac.secretKey = argv[2];

    char* startDate = "2017-01-01";
    char* endDate = "2017-01-02";
    char* granularity = "day";
    char* domains[2] = {"a.com","b.com"};

	Qiniu_Client_InitMacAuth(&cli, 1024, &mac);

    err = Qiniu_Cdn_GetBandwidthData(&cli,&ret,startDate,endDate,granularity,domains,2);

	printf("code:%d\n msg:%s\n", err.code, err.message);

	if (err.code == 200) {
		printf("-----------------------------------------\n");
		printf(" code : %d\n", ret.code);
		printf("error : %s\n", ret.error);
        printf(" time : %s\n", ret.time);
		printf(" data : %s\n", ret.data);
		printf("-----------------------------------------\n");
	}

	Qiniu_Client_Cleanup(&cli);
	//Qiniu_MacAuth_Cleanup();
	Qiniu_Servend_Cleanup();
	Qiniu_Global_Cleanup();

	return 0;
} // main
