/*
 ============================================================================
 Name        : up_demo_resumable.c
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
#include <unistd.h>
#include <time.h>

static const char* mimeType = NULL;

void try_terminate(const char* fl)
{
    char* prog_fl = QBox_String_Concat(fl, ".prog", NULL);
    unlink(prog_fl);
    free(prog_fl);
}

void try_save(const char* fl, QBox_UP_Progress* prog)
{
    char* prog_fl = QBox_String_Concat(fl, ".prog", NULL);
    FILE *fp = NULL;
    int i = 0;

    fp = fopen(prog_fl, "w");
    if (fp) {
        fprintf(fp, "blockCount=%d\n", prog->blockCount);
        fprintf(fp, "blockNextIndex=%d\n", prog->blockNextIndex);

        for (i = 0; i < prog->blockCount; ++i) {
            fprintf(fp, "checksum=");
            fwrite(prog->checksums[i].value, sizeof(prog->checksums[i].value), 1, fp);
            fprintf(fp, "\n");
        }

        for (i = 0; i < prog->blockCount; ++i) {
            fprintf(fp, "offset=%d\n", prog->progs[i].offset);
            fprintf(fp, "restSize=%d\n", prog->progs[i].restSize);

            if (prog->progs[i].ctx == NULL) {
                prog->progs[i].ctx = "";
            }
            fprintf(fp, "ctx=%s\n", prog->progs[i].ctx);
        }

        fclose(fp);
    }

    free(prog_fl);
}

void try_resume(const char* fl, QBox_UP_Progress* prog)
{
    char* prog_fl = QBox_String_Concat(fl, ".prog", NULL);
    char line[1024];
    FILE *fp = NULL;
    int i = 0;

    fp = fopen(prog_fl, "r");
    if (fp) {
        fscanf(fp, "blockCount=%d\n", &prog->blockCount);
        fscanf(fp, "blockNextIndex=%d\n", &prog->blockNextIndex);

        for (i = 0; i < prog->blockCount; ++i) {
            fseek(fp, strlen("checksum="), SEEK_CUR);
            fread(prog->checksums[i].value, sizeof(prog->checksums[i].value), 1, fp);
            fseek(fp, strlen("\n"), SEEK_CUR);
        }

        for (i = 0; i < prog->blockCount; ++i) {
            fscanf(fp, "offset=%d\n", &prog->progs[i].offset);
            fscanf(fp, "restSize=%d\n", &prog->progs[i].restSize);

            bzero(line, sizeof(line));
            fscanf(fp, "ctx=%s\n", line);
            prog->progs[i].ctx = strdup(line);
        }

        fclose(fp);

        printf("> Resumed %s!\n", prog_fl);
    }

    free(prog_fl);
}

typedef struct _QBox_Demo_Progress {
    QBox_UP_Progress* prog;
    const char* fl;
    int n;
    int m;
} QBox_Demo_Progress;

size_t get_timestamp(char* buf, size_t len)
{
    time_t now = time(NULL);
    bzero(buf, len);
    return strftime(buf, len, "%Y-%m-%d %H:%M:%S", localtime(&now));
}

int block_notify(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    QBox_Demo_Progress* demoProg = (QBox_Demo_Progress*) self;
    char ts[32];

    get_timestamp(ts, sizeof(ts));
    printf("%s : block_nofity : blockIdx=%d checksum=%28s\n", ts, blockIdx, checksum->value);

    try_save(demoProg->fl, demoProg->prog);
    
    if (blockIdx == demoProg->n) {
        return 0;
    }
    return 1;
}

int chunk_notify(void* self, int blockIdx, QBox_UP_BlockProgress* prog)
{
    QBox_Demo_Progress* demoProg = (QBox_Demo_Progress*) self;
    char ts[32];

    get_timestamp(ts, sizeof(ts));
    printf("%s : chunk_nofity : blockIdx=%d offset=%d restSize=%d errCode=%d ctx=[%s]\n",
            ts, blockIdx, prog->offset, prog->restSize, prog->errCode, prog->ctx);

    if (blockIdx == demoProg->n && demoProg->m >= 0 && prog->offset > demoProg->m) {
        try_save(demoProg->fl, demoProg->prog);
        return 0;
    }
    return 1;
}

void put_blocks(const char* fl, int n, int m)
{
    QBox_Error err;
    QBox_Client client;
    QBox_Client client2;
    QBox_AuthPolicy auth;
    QBox_ReaderAt f;
    QBox_UP_PutRet putRet;
    QBox_RS_GetRet getRet;
    char* uptoken = NULL;
    char* entry = NULL;
    QBox_UP_Progress* prog = NULL;
    QBox_Int64 fsize = 0;

    QBox_Demo_Progress demoProg;

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

    f = QBox_FileReaderAt_Open(fl);

    if ((int)f.self >= 0) {
        fsize = (QBox_Int64) lseek((int)f.self, 0, SEEK_END);

        prog = QBox_UP_NewProgress(fsize);

        demoProg.fl   = fl;
        demoProg.n    = n;
        demoProg.m    = m;
        demoProg.prog = prog;

        try_resume(fl, prog);

        printf("QBox_RS_ResumablePut\n");

        entry = QBox_String_Concat("Bucket:", fl, NULL);
        err = QBox_RS_ResumablePut(
            &client,
            &putRet,
            prog,
            block_notify,
            chunk_notify,
            &demoProg,   /* notifyParams   */
            entry,
            mimeType,
            f,
            fsize,
            NULL, /* customMeta     */
            NULL  /* callbackParams */
        );
        free(entry);

        QBox_FileReaderAt_Close(f.self);

        if (err.code != 200) {
            printf("QBox_RS_ResumablePut failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return;
        }
        else {
            try_terminate(fl);
        }

        QBox_UP_Progress_Release(prog);

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
    }

    QBox_Client_Cleanup(&client);

    printf("\n");
}

int main(int argc, const char* argv[])
{
    char* fl = argv[1];
    int n = -1;
    int m = -1;

    QBOX_ACCESS_KEY = "<Please apply your access key>";
    QBOX_SECRET_KEY = "<Dont send your secret key to anyone>";

    if (argc < 2) {
        printf("Usage: up_demo_resumable FILE [MIME] [ABORT_BLOCK_INDEX] [ABORT_CHUNK_BYTES]\n");
        return 0;
    }

    if (argc > 2) {
        mimeType = argv[2];
    }
    else {
        mimeType = "text/plain";
    }

    if (argc > 3) {
        n = atoi(argv[3]);

        if (n < 0) {
            printf("ABORT_BLOCK_INDEX must be zero or positive numbers!\n");
            return 0;
        }
    }

    if (argc > 4) {
        m = atoi(argv[4]);

        if (m < 0) {
            printf("ABORT_CHUNK_BYTES must be zero or positive numbers!\n");
            return 0;
        }
    }

    put_blocks(fl, n, m);

    QBox_Global_Cleanup();

    return 0;
}

