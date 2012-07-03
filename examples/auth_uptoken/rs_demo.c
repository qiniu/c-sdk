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
#include "../../qbox/oauth2.h"
#include "../../qbox/oauth2_uptoken.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	QBox_Error err;
	QBox_Client client;
	QBox_Client client2;
	QBox_RS_GetRet getRet;
    FILE* fp = NULL;
    char* uptoken = NULL;

	QBOX_ACCESS_KEY	= "<Please apply your access key>";
	QBOX_SECRET_KEY	= "<Dont send your secret key to anyone>";

	QBox_Global_Init(-1);

    {
        QBox_Zero(client2);
	    QBox_Client_Init(&client2, 1024);
	    printf("QBox_RS_Delete\n");
	    QBox_RS_Delete(&client2, "Bucket", "rs_demo.c");
        QBox_Client_Cleanup(&client2);
    }

	QBox_Zero(client);

	printf("QBox_Client_MakeUpToken\n");

    uptoken = QBox_Client_MakeUpToken("Bucket", 3600, NULL, NULL);
    if (uptoken == NULL) {
        printf("Cannot generate UpToken!\n");
        goto lzDone;
    }

	printf("QBox_Client_InitByUpToken\n");

	QBox_Client_InitByUpToken(&client, uptoken, 1024);

	printf("QBox_RS_PutFile_ByUpToken(rs_demo.c)\n");

    fp = fopen("./rs_demo.c", "r");
    if (fp) {
        err = QBox_RS_PutFile_ByUpToken(&client, "Bucket", "rs_demo.c", "text/plain", NULL, NULL, fp, 0);
        if (err.code != 200) {
		    printf("QBox_RS_PutFile_ByUpToken failed: %d - %s\n", err.code, err.message);
        }
        fclose(fp);
    }

	printf("QBox_RS_PutFile_ByUpToken(big_file.txt)\n");

    fp = fopen("./big_file.txt", "r");
    if (fp) {
        err = QBox_RS_PutFile_ByUpToken(&client, "Bucket", "big_file.txt", "text/plain", NULL, NULL, fp, 0);
        if (err.code != 200) {
		    printf("QBox_RS_PutFile_ByUpToken failed: %d - %s\n", err.code, err.message);
        }
        fclose(fp);
    }

	printf("QBox_RS_PutFile_ByUpToken(huge_file.txt)\n");

    fp = fopen("./huge_file.txt", "r");
    if (fp) {
        err = QBox_RS_PutFile_ByUpToken(&client, "Bucket", "huge_file.txt", "text/plain", NULL, NULL, fp, 0);
        if (err.code != 200) {
		    printf("QBox_RS_PutFile_ByUpToken failed: %d - %s\n", err.code, err.message);
        }
        fclose(fp);
    }

    {
        QBox_Zero(client2);
	    QBox_Client_Init(&client2, 1024);

        printf("QBox_RS_Get(rs_demo.c)\n");

        err = QBox_RS_Get(&client2, &getRet, "Bucket", "rs_demo.c", NULL);
        if (err.code != 200) {
            printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
            goto lzDone;
        }

        printf("Got url(rs_demo.c)=[%s]\n", getRet.url);
        printf("Got fsize(rs_demo.c)=%llu\n", getRet.fsize);

        printf("QBox_RS_Get(big_file.txt)\n");

        err = QBox_RS_Get(&client2, &getRet, "Bucket", "big_file.txt", NULL);
        if (err.code != 200) {
            printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
            goto lzDone;
        }

        printf("Got url(big_file.txt)=[%s]\n", getRet.url);
        printf("Got fsize(big_file.txt)=%llu\n", getRet.fsize);

        printf("QBox_RS_Get(huge_file.txt)\n");

        err = QBox_RS_Get(&client2, &getRet, "Bucket", "huge_file.txt", NULL);
        if (err.code != 200) {
            printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
            goto lzDone;
        }

        printf("Got url(huge_file.txt)=[%s]\n", getRet.url);
        printf("Got fsize(huge_file.txt)=%llu\n", getRet.fsize);

        QBox_Client_Cleanup(&client2);
    }

lzDone:
    free(uptoken);
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
	return 0;
}

