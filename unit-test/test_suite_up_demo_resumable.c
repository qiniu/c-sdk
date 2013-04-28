/*
 ============================================================================
 Name        : qbox_test.h
 Author      : Wu Shi Yu
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/TestDB.h>
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "../qbox/up.h"
#include "../qbox/oauth2.h"
#include "test.h"

//#define TESTFILE_16M "/home/wsy/文档/SDKUnitTest/src/test_file_16M.txt"
//#define TESTFILE_1M "/home/wsy/文档/SDKUnitTest/src/test_file_1M .txt"
#define TESTFILE_16M "test_file_16M.txt"
#define TESTFILE_1M "test_file_1M.txt"
#define MESSAGE_LEVEL 0
//MESSAGE_LEVEL: used to set the level of output message
static const char* mimeType = NULL;
QBox_Client *client_usedToDestroy;
char* uptoken_usedToDestroy = NULL;

static int mode;

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
    if(MESSAGE_LEVEL>=2)
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
    if(MESSAGE_LEVEL>=2)
        printf("%s : chunk_nofity : blockIdx=%d offset=%d restSize=%d errCode=%d ctx=[%s]\n",
            ts, blockIdx, prog->offset, prog->restSize, prog->errCode, prog->ctx);

    if (blockIdx == demoProg->n && demoProg->m >= 0 && prog->offset > demoProg->m) {
        try_save(demoProg->fl, demoProg->prog);
        return 0;
    }
    return 1;
}

int block_notify_err299(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    return 0;
}

int chunk_notify_error299_firstBlock(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    return 0;
}

static int Idx=-1;
int chunk_notify_error299_otherBlock(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    if(blockIdx==Idx){
        return 0;
    }
    Idx=blockIdx;
    return 1;
}
int chunk_notify_error401(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
	QBOX_SECRET_KEY = "err401";

    QBox_AuthPolicy auth;
    QBox_Zero(auth);

	uptoken_usedToDestroy = QBox_MakeUpToken(&auth);
	if (uptoken_usedToDestroy == NULL) {
		printf("Cannot generate UpToken!\n");
		return;
	}

	uptoken_usedToDestroy = QBox_String_Concat("Authorization: UpToken ", uptoken_usedToDestroy, NULL);
    client_usedToDestroy->auth.self = uptoken_usedToDestroy;

    return 1;
}

QBox_Error put_blocks(const char* fl, int n, int m,QBox_UP_FnBlockNotify blockNotify,QBox_UP_FnChunkNotify chunkNotify)
{
    QBox_Error err;
    QBox_Client client;
    client_usedToDestroy=&client;//used to test err401;
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

    if(MESSAGE_LEVEL>=1)
        printf("Processing ... %s\n", fl);

    /* Delete old file */
    QBox_Zero(client2);
    QBox_Client_Init(&client2, 1024);
    if(MESSAGE_LEVEL>=1)
        printf("QBox_RS_Delete\n");
    QBox_RS_Delete(&client2, "Bucket", fl);

    /* Upload file */
    QBox_Zero(client);
    QBox_Zero(auth);

    /* QBox_MakeUpToken() should be called on Biz-Server side */
    if(MESSAGE_LEVEL>=1)
        printf("QBox_MakeUpToken\n");

	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
		printf("Cannot generate UpToken!\n");
		return;
	}

    /* QBox_Client_InitByUpToken() and
     * other QBox_UP_xxx() functions should be called on Up-Client side */
    if(MESSAGE_LEVEL>=1)
        printf("QBox_Client_InitByUpToken\n");

    QBox_Client_InitByUpToken(&client, uptoken, 1024);

    uptoken_usedToDestroy=uptoken;//used to test err401

    f = QBox_FileReaderAt_Open(fl);

    CU_ASSERT_NOT_EQUAL(f.self,NULL);
    if (f.self != NULL) {
        fsize = (QBox_Int64) lseek((int)f.self, 0, SEEK_END);

        prog = QBox_UP_NewProgress(fsize);

        demoProg.fl   = fl;
        demoProg.n    = n;
        demoProg.m    = m;
        demoProg.prog = prog;

        try_resume(fl, prog);

        if(MESSAGE_LEVEL>=1)
            printf("QBox_RS_ResumablePut\n");

        entry = QBox_String_Concat("Bucket:", fl, NULL);
        err = QBox_RS_ResumablePut(
            &client,
            &putRet,
            prog,
            blockNotify,
            chunkNotify,
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
            if(MESSAGE_LEVEL>=1)
                printf("QBox_RS_ResumablePut failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return err;
        }
        else {
            try_terminate(fl);
        }

        QBox_UP_Progress_Release(prog);

        /* Check uploaded file */
        if(MESSAGE_LEVEL>=1)
            printf("QBox_RS_Get\n");

        err = QBox_RS_Get(&client2, &getRet, "Bucket", fl, NULL);

        if (err.code != 200) {
            if(MESSAGE_LEVEL>=1)
                printf("QBox_RS_Get failed: %d - %s\n", err.code, err.message);
            free(uptoken);
            return err;
        }

        CU_ASSERT_EQUAL(getRet.fsize,fsize);

        if(MESSAGE_LEVEL>=2)
            printf("Got url=[%s]\n", getRet.url);
        if(MESSAGE_LEVEL>=2)
            printf("Got fsize=%llu\n", getRet.fsize);

        QBox_RS_Delete(&client2, "Bucket", fl);

        QBox_Client_Cleanup(&client2);
    }

    QBox_Client_Cleanup(&client);
    return err;
}


void test_by_up_demo_resumable_16M(){
    //mimeType = "text/plain";
    QBox_Error err;
    err=put_blocks(TESTFILE_16M, -1, -1,block_notify,chunk_notify);
    CU_ASSERT_EQUAL(err.code,200);
}
void test_by_up_demo_resumable_1M(){
    //mimeType = "text/plain";
    QBox_Error err;
    err=put_blocks(TESTFILE_1M, -1, -1,block_notify,chunk_notify);
    CU_ASSERT_EQUAL(err.code,200);
}

void test_by_up_demo_resumable_err299(){
    mimeType = "text/plain";
    QBox_Error err;
    err=put_blocks(TESTFILE_16M, -1, -1,block_notify_err299,chunk_notify);
    CU_ASSERT_EQUAL(err.code,299);
    err=put_blocks(TESTFILE_1M, -1, -1,block_notify_err299,chunk_notify);
    CU_ASSERT_EQUAL(err.code,200);
    err=put_blocks(TESTFILE_16M, -1, -1,block_notify,chunk_notify_error299_firstBlock);
    CU_ASSERT_EQUAL(err.code,299);
    err=put_blocks(TESTFILE_16M, -1, -1,block_notify,chunk_notify_error299_otherBlock);
    CU_ASSERT_EQUAL(err.code,299);
}
void test_by_up_demo_resumable_err18(){
    if((mode&ADD_BAD_TEST)==0)
        return;
    mimeType = "text/plain";
    QBox_Error err;
    err=put_blocks(TESTFILE_16M, -1, -1,block_notify,chunk_notify_error401);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,18);
    if(err.code!=18){
        printf("\n\n\nerr %d:%s\n\n",err.code,err.message);
    }
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";


	QBOX_SECRET_KEY = "err401";
    err=put_blocks(TESTFILE_16M, -1, -1,block_notify,chunk_notify);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,18);
    if(err.code!=18){
        printf("\n\n\nerr %d:%s\n\n",err.code,err.message);
    }
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";
}
/*
void test_QBox_UP_NewProgress(){
    QBox_UP_Progress* prog = NULL;
    prog = QBox_UP_NewProgress(4194304);
    CU_ASSERT_EQUAL(prog->blockCount,1);
    CU_ASSERT_EQUAL(prog->progs[0].restSize,4194304);
    QBox_UP_Progress_Release(prog);
    prog = QBox_UP_NewProgress(4194303);
    CU_ASSERT_EQUAL(prog->blockCount,1);
    CU_ASSERT_EQUAL(prog->progs[0].restSize,4194303);
    QBox_UP_Progress_Release(prog);

    prog = QBox_UP_NewProgress(4194305);
    CU_ASSERT_EQUAL(prog->blockCount,2);
    CU_ASSERT_EQUAL(prog->progs[0].restSize,4194304);
    CU_ASSERT_EQUAL(prog->progs[1].restSize,1);
    QBox_UP_Progress_Release(prog);
    prog=NULL;
    QBox_UP_Progress_Release(prog);
}
*/

/**//*---- test suites ------------------*/
int suite_init_up_demo_resumable(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	return 0;
}

int suite_clean_up_demo_resumable(void)
{
	QBox_Global_Cleanup();
    return 0;
}

QBOX_TESTS_BEGIN(up_demo_resumable)
QBOX_TEST(test_by_up_demo_resumable_16M)
QBOX_TEST(test_by_up_demo_resumable_1M)
QBOX_TEST(test_by_up_demo_resumable_err299)
QBOX_TEST(test_by_up_demo_resumable_err18)
//QBOX_TEST(test_QBox_UP_NewProgress)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(up_demo_resumable,suite_init_up_demo_resumable,suite_clean_up_demo_resumable)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsUpDemoResumable(int myMode)
{
        mode=myMode;
        QBOX_TEST_REGISTE(up_demo_resumable)
}
