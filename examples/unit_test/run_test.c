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

	QBOX_ACCESS_KEY = getenv("QINIU_ACCESS_KEY");
	if (QBOX_ACCESS_KEY == NULL) {
		printf("找不到环境变量: QINIU_ACCESS_KEY\n");
		return;
	}

	QBOX_SECRET_KEY = getenv("QINIU_SECRET_KEY");
	if (QBOX_SECRET_KEY == NULL) {
		printf("找不到环境变量: QINIU_SECRET_KEY\n");
		return;
	}

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	QBT_Do(&client);

	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

	return 0;
}


