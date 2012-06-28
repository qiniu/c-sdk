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

<<<<<<< HEAD:examples/rs_accesskey_demo.c
	QBox_Client_Init_ByAccessKey(&client, accessKey, secretKey, 1024);
	QBox_RS_Delete(&client, "Bucket", "rs_accesskey_demo.c");

	printf("QBox_RS_PutFile\n");

	err = QBox_RS_PutFile(&client, &putRet, "Bucket", "rs_accesskey_demo.c", "application/octet-stream", __FILE__, "");
=======
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
>>>>>>> xusw/master:examples/auth_passwd/rs_demo.c
	if (err.code != 200) {
		printf("QBox_RS_PutFile failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Get\n");

<<<<<<< HEAD:examples/rs_accesskey_demo.c
	err = QBox_RS_Get(&client, &getRet, "Bucket", "rs_accesskey_demo.c", NULL);
=======
	err = QBox_RS_Get(&client, &getRet, "tblName", "rs/demo.c", NULL);
>>>>>>> xusw/master:examples/auth_passwd/rs_demo.c
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

<<<<<<< HEAD:examples/rs_accesskey_demo.c
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, "Bucket", "rs_accesskey_demo.c", "application/octet-stream", __FILE__, "", "key=rs_accesskey_demo.c");
=======
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, "tblName", "rs_demo.c", "application/octet-stream", __FILE__, "", "key=rs_demo.c");
>>>>>>> xusw/master:examples/auth_passwd/rs_demo.c
	if (err.code != 200) {
		printf("QBox_RS_PutFile failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Get\n");

<<<<<<< HEAD:examples/rs_accesskey_demo.c
	err = QBox_RS_Get(&client, &getRet, "Bucket", "rs_accesskey_demo.c", NULL);
=======
	err = QBox_RS_Get(&client, &getRet, "tblName", "rs_demo.c", NULL);
>>>>>>> xusw/master:examples/auth_passwd/rs_demo.c
	if (err.code != 200) {
		printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}
	hash = strdup(getRet.hash);

	printf("QBox_RS_GetIfNotModified: %s\n", hash);

<<<<<<< HEAD:examples/rs_accesskey_demo.c
	err = QBox_RS_GetIfNotModified(&client, &getRet, "Bucket", "rs_accesskey_demo.c", NULL, hash);
=======
	err = QBox_RS_GetIfNotModified(&client, &getRet, "tblName", "rs_demo.c", NULL, hash);
>>>>>>> xusw/master:examples/auth_passwd/rs_demo.c
	free(hash);
	if (err.code != 200) {
		printf("QBox_RS_GetIfNotModified failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Stat\n");

<<<<<<< HEAD:examples/rs_accesskey_demo.c
	err = QBox_RS_Stat(&client, &statRet, "Bucket", "rs_accesskey_demo.c");
=======
	err = QBox_RS_Stat(&client, &statRet, "tblName", "rs_demo.c");
>>>>>>> xusw/master:examples/auth_passwd/rs_demo.c
	if (err.code != 200) {
		printf("QBox_RS_Stat failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Publish\n");

	err = QBox_RS_Publish(&client, "Bucket", "iovip.qbox.me/Bucket");
	if (err.code != 200) {
		printf("QBox_RS_Publish failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Unpublish\n");

	err = QBox_RS_Unpublish(&client, "iovip.qbox.me/Bucket");
	if (err.code != 200) {
		printf("QBox_RS_Unpublish failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Delete\n");

<<<<<<< HEAD:examples/rs_accesskey_demo.c
	err = QBox_RS_Delete(&client, "Bucket", "rs_accesskey_demo.c");
=======
	err = QBox_RS_Delete(&client, "tblName", "rs_demo.c");
>>>>>>> xusw/master:examples/auth_passwd/rs_demo.c
	if (err.code != 200) {
		printf("QBox_RS_Delete failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Drop\n");

	err = QBox_RS_Drop(&client, "Bucket");
	if (err.code != 200) {
		printf("QBox_RS_Drop failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

lzDone:
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
	return 0;
}

