#include "../../qbox/rs.h"
#include "../../qbox/up.h"
#include "../../qbox/wm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() 
{
	QBox_Error err;
	QBox_Client client;
	char* uptoken;


	QBOX_ACCESS_KEY		= "<Please apply your access key>";
	QBOX_SECRET_KEY		= "<Dont send your secret key to anyone>";


	char* customer = "";
	char* Bucket = "";
	char* Domain = "";
	char* key = "test.jpg";
	char* imgfile = "test.jpg";
	char* imgType = "image/jpg";

	QBox_AuthPolicy Auth = 
		{Bucket, NULL, NULL, customer, 3600};
	QBox_WM_Template tpl = 
		{NULL, 0, NULL, "hello", Bucket, NULL, NULL, 1, 19};

	QBox_Zero(client);
	QBox_Global_Init(-1);

	printf("QBox_Client_Init_ByAccessKey\n");

	QBox_Client_Init(&client, 1024);

	printf("QBox_WM_Set\n");

	err = QBox_WM_Set(&client, &tpl, customer);
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

	printf("QBox_RS_Mkbucket\n");
	err = QBox_RS_Mkbucket(&client, Bucket);
	if (err.code != 200) {
		printf("QBox_RS_Mkbucket failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_SetProtected\n");

	err = QBox_RS_SetProtected(&client, Bucket, 1);
	if (err.code != 200) {
		printf("QBox_RS_SetProtected failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_SetSeparator\n");
	err = QBox_RS_SetSeparator(&client, Bucket, "-");
	if (err.code != 200) {
		printf("QBox_RS_SetSeparator failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_SetStyle\n");
	err = QBox_RS_SetStyle(&client, Bucket, "midWm", "imageView/0/w/480/h/800/watermark/1");
	if (err.code != 200) {
		printf("QBox_RS_SetStyle failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_SetStyle\n");
	err = QBox_RS_SetStyle(&client, Bucket, "midNoWm", "imageView/0/w/480/h/800/watermark/0");
	if (err.code != 200) {
		printf("QBox_RS_SetStyle failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	printf("QBox_RS_UnsetStyle\n");
	err = QBox_RS_UnsetStyle(&client, Bucket, "midNoWm");
	if (err.code != 200) {
		printf("QBox_RS_UnsetStyle failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

	QBox_RS_Delete(&client, Bucket, key);

	printf("QBox_UP_UploadFile\n");
	uptoken = QBox_MakeUpToken(&Auth);
	err = QBox_UP_UploadFile(NULL, uptoken, Bucket, key, imgType, imgfile, NULL, NULL, NULL);
	if (err.code != 200) {
		printf("QBox_UP_UploadFile failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}
	free(uptoken);

	printf("QBox_RS_Publish\n");

	err = QBox_RS_Publish(&client, Bucket, Domain);
	if (err.code != 200) {
		printf("QBox_RS_Publish failed: %d - %s\n", err.code, err.message);
		goto lzDone;
	}

lzDone:
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
	return 0;
}
