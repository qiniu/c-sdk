/*
 ============================================================================
 Name        : up_demo.c
 Author      : RS Author
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : RS Demo
 ============================================================================
 */

#include "../../qbox/rs.h"
#include "../../qbox/up.h"
#include "../../qbox/oauth2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ssize_t read_chunk(void* self, void *buf, size_t bytes, off_t offset)
{
    FILE* fp = (FILE*) self;

    fseek(fp, offset, SEEK_SET);
    if (fread(buf, bytes, 1, fp) == 1) {
        return bytes;
    }

    return 0;
}

void put_blocks(const char* fl)
{
    QBox_Error err;
    QBox_Client client;
    QBox_Client client2;
    QBox_AuthPolicy auth;
    QBox_ReaderAt f;
    QBox_RS_GetRet getRet;
    FILE* fp = NULL;
    char* uptoken = NULL;
    char* entry = NULL;
    QBox_Json* root = NULL;
    QBox_UP_Progress* prog = NULL;
    QBox_Int64 fsize = 0;

    printf("Processing ... %s\n", fl);

    /* Delete old file */
    QBox_Zero(client2);
    QBox_Client_Init(&client2, 1024);
    printf("QBox_RS_Delete\n");
    QBox_RS_Delete(&client2, "Bucket", fl);

    /* Upload file */
    QBox_Zero(client);
    QBox_Zero(auth);

    /* QBox_MakeUpToken() should be called on Biz-Server side */
	printf("QBox_MakeUpToken\n");

	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
		printf("Cannot generate UpToken!\n");
		return;
	}

    /* QBox_Client_InitByUpToken() and 
     * other QBox_UP_xxx() functions should be called on Up-Client side */
    printf("QBox_Client_InitByUpToken\n");

    QBox_Client_InitByUpToken(&client, uptoken, 1024);

    fp = fopen(fl, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        f.self = fp;
        f.ReadAt = read_chunk;

        prog = QBox_UP_NewProgress(fsize);

        printf("QBox_UP_Put\n");

        err = QBox_UP_Put(&client, f, fsize, prog, NULL, NULL, NULL);
        fclose(fp);

        if (err.code != 200) {
            printf("QBox_UP_Put failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return;
        }

        printf("QBox_UP_Mkfile\n");

        entry = QBox_String_Concat("Bucket:", fl, NULL);
        err = QBox_UP_Mkfile(
            &client,
            &root,
            "/rs-mkfile/",
            entry,
            "text/plain",
            fsize,
            NULL,
            NULL,
            prog->checksums,
            prog->blockCount
        );
        free(entry);

        if (err.code != 200) {
            printf("QBox_UP_Put failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return;
        }

        QBox_UP_Progress_Release(prog);
    }

    /* Check uploaded file */
    printf("QBox_RS_Get\n");

    err = QBox_RS_Get(&client2, &getRet, "Bucket", fl, NULL);
    if (err.code != 200) {
        printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
        free(uptoken);
        return;
    }

    printf("Got url=[%s]\n", getRet.url);
    printf("Got fsize=%llu\n", getRet.fsize);

    QBox_Client_Cleanup(&client2);
    QBox_Client_Cleanup(&client);

    printf("\n");
}

int main()
{
    //QBOX_ACCESS_KEY = "<Please apply your access key>";
    //QBOX_SECRET_KEY = "<Dont send your secret key to anyone>";

    QBOX_ACCESS_KEY = "RLT1NBD08g3kih5-0v8Yi6nX6cBhesa2Dju4P7mT";
    QBOX_SECRET_KEY = "k6uZoSDAdKBXQcNYG3UOm4bP3spDVkTg-9hWHIKm";

    QBox_Global_Init(-1);

    put_blocks("up_demo.c");
    put_blocks("big_file.txt");

    /* Generate a file first which size is more than 4MB and name it as 'huge_file.txt' */
    put_blocks("huge_file.txt");

    QBox_Global_Cleanup();

    return 0;
}

