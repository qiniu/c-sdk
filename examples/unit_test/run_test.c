/*
 ============================================================================
 Name        : run_test.c
 Author      : Jiang Wen Long
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */

#include "stdlib.h"
#include "qbox_test.h"
#include "../../qbox/rs.h"

int main()
{
	QBox_Error err;
	QBox_Client client;

	printf("如果运行出错且错误码为401，请将QINIU_ACCESS_KEY和QINIU_SECRET_KEY加到环境变量中\n");

	QBOX_ACCESS_KEY = getenv("QINIU_ACCESS_KEY");
	QBOX_SECRET_KEY = getenv("QINIU_SECRET_KEY");

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	QBT_Do(&client);

	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

	return 0;
}


