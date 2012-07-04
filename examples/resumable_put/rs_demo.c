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
#include "../../qbox/up.h"
#include "../../qbox/oauth2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_block_progress(void* data,
    int blockIndex, const char* ctx, const char* cksum,
    QBox_Off_t chunkOffset, QBox_Error err)
{
    printf("blockIndex=%d\n", blockIndex);
    printf("ctx=%s\n", ctx);
    printf("cksum=%s\n", cksum);
    printf("chunkOffset=%lu\n", chunkOffset);
    printf("\n");
}

int read_chunk(void* data, size_t offset, void* chunk, size_t* size)
{
    FILE* fp = (FILE*) data;

    if (fread(chunk, *size, 1, fp) == 1) {
        return *size;
    }

    return 0;
}

void put_blocks(const char* fl)
{
    QBox_Error err;
    QBox_Client client;
    QBox_Client client2;
    QBox_AuthPolicy auth;
    QBox_RS_BlockIO io;
    QBox_RS_BlockProgress prog;
    QBox_RS_GetRet getRet;
    FILE* fp = NULL;
    char* uptoken = NULL;

    printf("Processing ... %s\n", fl);

    /* Delete old file */
    QBox_Zero(client2);
    QBox_Client_Init(&client2, 1024);
    printf("QBox_RS_Delete\n");
    QBox_RS_Delete(&client2, "Bucket", fl);

    /* Upload file */
    QBox_Zero(client);
    QBox_Zero(auth);

	printf("QBox_MakeUpToken\n");

	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
		printf("Cannot generate UpToken!\n");
		return;
	}

    printf("QBox_Client_InitByUpToken\n");

    QBox_Client_InitByUpToken(&client, uptoken, 1024);

    printf("QBox_RS_PutFile_ByUpToken\n");

    fp = fopen(fl, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);

        io.readChunk = read_chunk;
        io.data = (void *)fp;
        io.fileSize = ftell(fp);
        io.chunkSize = 0;

        fseek(fp, 0, SEEK_SET);

        prog.callback = check_block_progress; 
        prog.data = NULL;

        err = QBox_RS_PutFile_ByUpToken(&client, "Bucket", fl, "text/plain", NULL, NULL, &io, &prog);
        fclose(fp);

        if (err.code != 200) {
            printf("QBox_RS_PutFile_ByUpToken failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return;
        }
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
    QBOX_ACCESS_KEY = "<Please apply your access key>";
    QBOX_SECRET_KEY = "<Dont send your secret key to anyone>";

    QBox_Global_Init(-1);

    put_blocks("rs_demo.c");
    put_blocks("big_file.txt");

    /* Generate a file first which size is more than 4MB and name it as 'huge_file.txt' */
    put_blocks("huge_file.txt");

    QBox_Global_Cleanup();

    return 0;
}

