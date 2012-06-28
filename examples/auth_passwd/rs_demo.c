/*
 ============================================================================
 Name        : rs_demo.c
 Author      : RS Author
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : RS Demo
 ============================================================================
 */

#include "../../qbox/rs.h"
#include "../../qbox/rscli.h"
#include "../../qbox/oauth2_passwd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	QBox_Error err;
	QBox_Token* token;
	QBox_Client client;
	QBox_RS_PutAuthRet putAuthRet;
	QBox_RS_PutRet putRet;
	QBox_RS_GetRet getRet;
	QBox_RS_StatRet statRet;
	char* hash;

	QBox_Zero(client);
	QBox_Global_Init(-1);

	printf("QBox_Token_ExchangeByPassword\n");

	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	if (err.code != 200) {
		printf("QBox_Token_ExchangeByPassword failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	QBox_Client_InitByPassword(&client, token, 1024);
	QBox_Token_Release(token);
	QBox_RS_Delete(&client, "tblName", "rs_demo.c");

	printf("QBox_RS_PutFile\n");

	err = QBox_RS_PutFile(&client, &putRet, "tblName", "rs/demo.c", "application/octet-stream", __FILE__, "");
	if (err.code != 200) {
		printf("QBox_RS_PutFile failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Get\n");

	err = QBox_RS_Get(&client, &getRet, "tblName", "rs/demo.c", NULL);
	if (err.code != 200) {
		printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}
	
	printf("QBox_RS_PutAuth\n");

	err = QBox_RS_PutAuth(&client, &putAuthRet);
	if (err.code != 200) {
		printf("QBox_RS_PutAuth failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RSCli_PutFile\n");

	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, "tblName", "rs_demo.c", "application/octet-stream", __FILE__, "", "key=rs_demo.c");
	if (err.code != 200) {
		printf("QBox_RS_PutFile failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Get\n");

	err = QBox_RS_Get(&client, &getRet, "tblName", "rs_demo.c", NULL);
	if (err.code != 200) {
		printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}
	hash = strdup(getRet.hash);

	printf("QBox_RS_GetIfNotModified: %s\n", hash);

	err = QBox_RS_GetIfNotModified(&client, &getRet, "tblName", "rs_demo.c", NULL, hash);
	free(hash);
	if (err.code != 200) {
		printf("QBox_RS_GetIfNotModified failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Stat\n");

	err = QBox_RS_Stat(&client, &statRet, "tblName", "rs_demo.c");
	if (err.code != 200) {
		printf("QBox_RS_Stat failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Publish\n");

	err = QBox_RS_Publish(&client, "tblName", "iovip.qbox.me/tblName");
	if (err.code != 200) {
		printf("QBox_RS_Publish failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Unpublish\n");

	err = QBox_RS_Unpublish(&client, "iovip.qbox.me/tblName");
	if (err.code != 200) {
		printf("QBox_RS_Unpublish failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Delete\n");

	err = QBox_RS_Delete(&client, "tblName", "rs_demo.c");
	if (err.code != 200) {
		printf("QBox_RS_Delete failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Drop\n");

	err = QBox_RS_Drop(&client, "tblName");
	if (err.code != 200) {
		printf("QBox_RS_Drop failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

lzDone:
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
	return 0;
}

