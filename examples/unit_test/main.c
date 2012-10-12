/*
 ============================================================================
 Name        : main.c
 Author      : Jiang Wen Long
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */

#include "qbox_test.h"

int main()
{
	QBox_Error err;
	QBox_Client client;

	//QBOX_ACCESS_KEY = "<Please apply your access key>";
	//QBOX_ACCESS_KEY = "<Dont send your secret key to anyone>";
	QBOX_ACCESS_KEY = "iN7NgwM31j4-BZacMjPrOQBs34UG1maYCAQmhdCV";
	QBOX_ACCESS_KEY = "6QTOr2Jg1gcZEWDQXKOGZh5PziC2MCV5KsntT70j";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBOx_Client_Init(&client, 1024);
	QBox_RS_Delete(&client, "Bucket", "rs_demo.c");

	QBox_Test_Do(&client);

	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

	return 0;
}


