#include "../../qbox/rs.h"
#include "../../qbox/rscli.h"
#include "../../qbox/wm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() 
{
	QBox_Error err;
	QBox_Client client;
	QBox_AuthPolicy Auth;
	QBox_RS_PutAuthRet putAuthRet;
	QBox_WM_Template tpl = 
		{"Sans",0,NULL,"hello","","5000","8",1,19};
	char* uptoken;


	QBOX_ACCESS_KEY		= "<Please apply your access key>";
	QBOX_SECRET_KEY		= "<Dont send your secret key to anyone>";


	static char* customer = "<...>";
	static char* Bucket = "<...>";
	static char* Domain = "<...>";
	static char* key = "kebi";
	static char* localFile = "kebi.jpg";


	QBox_Zero(client);
	QBox_Global_Init(-1);

	printf("QBox_Client_Init_ByAccessKey\n");

	QBox_Client_Init(&client, 1024);



	printf("QBox_WM_Set\n");

	err = QBox_WM_Set(&client, &tpl,"");
	if (err.code != 200) {
		printf("QBox_WM_Set failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	QBox_WM_Template ntpl;
	printf("QBox_WM_Get\n");

	err = QBox_WM_Get(&client, customer, &ntpl);
	if (err.code != 200) {
		printf("QBox_WM_Get failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_SetProtected\n");

	err = QBox_RS_SetProtected(&client, Bucket, 0);
	if (err.code != 200) {
		printf("QBox_RS_SetProtected: %d - %s\n", err.code, err.message);
		goto lzDone;
	}


	printf("QBox_RS_SetSeparator\n");
	err = QBox_RS_SetSeparator(&client, Bucket, "-");
	if (err.code != 200) {
		printf("QBox_RS_SetSeparator: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_SetStyle\n");
	err = QBox_RS_SetStyle(&client, Bucket, "midWm", "imageView/0/w/128/h/128/watermark/1");
	if (err.code != 200) {
		printf("QBox_RS_SetStyle: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_SetStyle\n");
	err = QBox_RS_SetStyle(&client, Bucket, "midNoWm", "imageView/0/w/128/h/128/watermark/0");
	if (err.code != 200) {
		printf("QBox_RS_SetStyle: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_UnsetStyle\n");
	err = QBox_RS_UnsetStyle(&client, Bucket, "midNoWm");
	if (err.code != 200) {
		printf("QBox_RS_UnsetStyle: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_Put\n");

	err = QBox_RS_PutAuth(&client, &putAuthRet);
	if (err.code != 200) {
		printf("QBox_RS_PutAuth failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}
	Auth.scope = Bucket;
	Auth.expires = putAuthRet.expiresIn;
	Auth.customer = customer;
	Auth.callbackUrl = Auth.returnUrl = NULL;
	uptoken = QBox_MakeUpToken(&Auth);
	if (err.code != 200) {
		printf("QBox_MakeUpToken failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, Bucket,
	 key, "application/octet-stream", localFile, "", "", uptoken);
	if (err.code != 200) {
		printf("QBox_RSCli_PutFile failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}
	free(uptoken);

	printf("QBox_RS_Delete\n");
	err = QBox_RS_Delete(&client, Bucket, key);
	if (err.code != 200) {
		printf("QBox_RS_Delete failed: %d - %s\n", err.code, err.message);
	}

lzDone:
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
	return 0;
}
