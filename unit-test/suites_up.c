/*
 ============================================================================
 Name        : suites_up.c
 Author      : Wu Shi Yu
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/up.h"
#include "../qbox/base.h"
#include "test.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE "/home/wsy/文档/c-sdk/unit-test/test_file.txt"
//#define TESTFILE_16M "/home/wsy/文档/c-sdk/unit-test/test_file_16M.txt"
#define TESTFILE "test_file.txt"
#define TESTFILE_16M "test_file_16M.txt"

void test_QBox_UP_Mkblock(){
    QBox_UP_PutRet* ret=malloc(sizeof(QBox_UP_PutRet));
    int blkSize;
    QBox_Reader body;
    int bodyLength;

    FILE* file;
    file=fopen(TESTFILE,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    body=QBox_FILE_Reader(file);
    bodyLength=29;
    blkSize=65536;
    err=QBox_UP_Mkblock(&client,ret,blkSize,body,bodyLength);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(ret->crc32,1715032928);
    fclose(file);
    free(ret);
    //Test Error
    //test error401
    ret=malloc(sizeof(QBox_UP_PutRet));
    file=fopen(TESTFILE,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    body=QBox_FILE_Reader(file);
    bodyLength=29;
    blkSize=65536;
    QBOX_ACCESS_KEY = "error";
    err=QBox_UP_Mkblock(&client,ret,blkSize,body,bodyLength);
    CU_ASSERT_EQUAL(err.code,401);
    fclose(file);
    free(ret);
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
}

void test_QBox_UP_Blockput(){
    QBox_UP_PutRet* putRet=malloc(sizeof(QBox_UP_PutRet));
    int blkSize;
    QBox_Reader body;
    int fsize;
    QBox_UP_BlockProgress *blkProg;

    FILE* file;
    file=fopen(TESTFILE_16M,"rb");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
	fseek(file, 0, SEEK_END);
	fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
    body=QBox_FILE_Reader(file);

    blkSize=65536;
    //err=QBox_UP_Mkblock(&client,putRet,blkSize,body,bodyLength);
    CU_ASSERT_EQUAL(err.code,200);
    fclose(file);
    free(putRet);
}
QBox_Error test_qbox_up_resumableblock(QBox_UP_PutRet* ret,int blkIndex,QBox_UP_Progress* prog,QBox_UP_FnChunkNotify chunkNotify){
    const char* fl=TESTFILE_16M;
    QBox_ReaderAt f;
    f = QBox_FileReaderAt_Open(fl);
    return QBox_UP_ResumableBlockput(
        &client,ret,f,
        blkIndex,
        prog->progs[blkIndex].offset + prog->progs[blkIndex].restSize,
        QBOX_PUT_CHUNK_SIZE,
        QBOX_PUT_RETRY_TIMES,
        &prog->progs[blkIndex],
        chunkNotify,
        NULL
        );
}
QBox_Error test_qbox_up_resumableblock_err(QBox_UP_FnChunkNotify chunkNotify){
    QBox_UP_PutRet* ret=malloc(sizeof(QBox_UP_PutRet));
    const char* fl=TESTFILE_16M;
    QBox_ReaderAt f;
    f = QBox_FileReaderAt_Open(fl);
    int blkIndex=0;
    QBox_UP_Progress* prog = QBox_UP_NewProgress(QBOX_PUT_CHUNK_SIZE*2);
    return QBox_UP_ResumableBlockput(
        &client,ret,f,
        blkIndex,
        prog->progs[blkIndex].offset + prog->progs[blkIndex].restSize,
        QBOX_PUT_CHUNK_SIZE,
        0,
        &prog->progs[blkIndex],
        chunkNotify,
        NULL
        );
}
int times;
int chunk_notify_up(void* self, int blockIdx, QBox_UP_BlockProgress* prog)
{
    times++;
    return 1;
}
int chunk_notify_up_err299_1(void* self, int blockIdx, QBox_UP_BlockProgress* prog)
{
    return 0;
}
int chunk_notify_up_err299_2(void* self, int blockIdx, QBox_UP_BlockProgress* prog)
{
    if(prog->restSize==QBOX_PUT_CHUNK_SIZE)
        return 1;
    return 0;
}
int chunk_notify_up_err18_1(void* self, int blockIdx, QBox_UP_BlockProgress* prog)
{
    QBOX_ACCESS_KEY = "err18";
	QBOX_SECRET_KEY = "err18";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);
    return 1;
}

void test_QBox_UP_ResumableBlockput(){
    QBox_UP_PutRet* putRet=malloc(sizeof(QBox_UP_PutRet));
    QBox_UP_Progress* prog = NULL;
    int blkIndex;

    //test branch: blockNotify=NULL
    blkIndex=0;
    prog = QBox_UP_NewProgress(QBOX_PUT_CHUNK_SIZE*2);
    err=test_qbox_up_resumableblock(putRet,blkIndex,prog,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    //test branch: blockNotify!=NULL
    blkIndex=0;
    times=0;
    prog = QBox_UP_NewProgress(QBOX_PUT_CHUNK_SIZE*2);
    err=test_qbox_up_resumableblock(putRet,blkIndex,prog,chunk_notify_up);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(times,2);
    //test branch: blockNotify!=NULL
    blkIndex=0;
    prog = QBox_UP_NewProgress(QBOX_PUT_CHUNK_SIZE*2);
    err=test_qbox_up_resumableblock(putRet,blkIndex,prog,NULL);
    err=test_qbox_up_resumableblock(putRet,blkIndex,prog,NULL);
    CU_ASSERT_EQUAL(err.code,200);
    //test Error
    //test error299 (up.c 133)
    blkIndex=0;
    prog = QBox_UP_NewProgress(QBOX_PUT_CHUNK_SIZE*2);
    err=test_qbox_up_resumableblock(putRet,blkIndex,prog,chunk_notify_up_err299_1);
    CU_ASSERT_EQUAL(err.code,299);
    //test error299 (up.c 185)
    blkIndex=0;
    prog = QBox_UP_NewProgress(QBOX_PUT_CHUNK_SIZE*2);
    err=test_qbox_up_resumableblock(putRet,blkIndex,prog,chunk_notify_up_err299_2);
    CU_ASSERT_EQUAL(err.code,299);
    //test err
    if((myMode&ADD_BAD_TEST)!=0){
    err= test_qbox_up_resumableblock_err(chunk_notify_up_err18_1);
    CU_ASSERT_EQUAL(err.code,18);
    printf("\n%d %s\n",err.code,err.message);
    err= test_qbox_up_resumableblock_err(NULL);
    CU_ASSERT_EQUAL(err.code,18);
    printf("\n%d %s\n",err.code,err.message);
    suite_init_up();
    }


}

void test_QBox_UP_Mkfile(){
    QBox_UP_PutRet* putRet=malloc(sizeof(QBox_UP_PutRet));
    const char* fl=TESTFILE;
    QBox_ReaderAt f;
    QBox_Int64 fsize=29;
    QBox_UP_Progress* prog = NULL;
    //test branch: blockNotify=NULL
    f = QBox_FileReaderAt_Open(fl);
    prog = QBox_UP_NewProgress(29);
    err = QBox_UP_Put(&client,putRet, f, fsize, prog, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(err.code,200);

    QBox_Json* root = NULL;
    char* params = NULL;
    const char* callbackParams=NULL;
    const char* mimeType=NULL;
    const char* entryURI=QBox_String_Concat("Bucket:", "test_up", NULL);

    err = QBox_UP_Mkfile(
        &client,
        &root,
        "/rs-mkfile/",
        entryURI,
        mimeType,
        fsize,
        params,
        callbackParams,
        prog->checksums,
        prog->blockCount
    );
    CU_ASSERT_EQUAL(err.code,200);

    params = "";
    callbackParams="";
    mimeType="application/octet-stream";
    entryURI=QBox_String_Concat("Bucket:", "test_up", NULL);

    err = QBox_UP_Mkfile(
        &client,
        &root,
        "/rs-mkfile/",
        entryURI,
        mimeType,
        fsize,
        params,
        callbackParams,
        prog->checksums,
        prog->blockCount
    );
    CU_ASSERT_EQUAL(err.code,200);
    QBox_RS_Drop(&client,"test_up");
}

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
}

void test_QBox_UP_Progress_Release(){
    QBox_UP_Progress* prog = NULL;
    prog = QBox_UP_NewProgress(4194304);
    QBox_UP_Progress_Release(prog);
    prog=NULL;
    QBox_UP_Progress_Release(prog);
}
int* temp;
int temp_index=0;
int block_notify_findnextblock(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    *(temp+temp_index)=blockIdx;
    temp_index++;
    return 1;
}
void test_qbox_up_findnextblock(QBox_UP_PutRet* putRet,QBox_Int64 fsize,QBox_UP_Progress* prog){
    const char* fl=TESTFILE_16M;
    QBox_ReaderAt f;
    f = QBox_FileReaderAt_Open(fl);
    err = QBox_UP_Put(&client,putRet, f, fsize, prog, block_notify_findnextblock, NULL, NULL);
}

void test_QBox_UP_FindNextBlock(){
    QBox_UP_PutRet* putRet=malloc(sizeof(QBox_UP_PutRet));
    QBox_UP_Progress* prog = NULL;
    //test branch: progs[i].ctx=null
    temp=malloc(64);
    temp_index=0;
    prog = QBox_UP_NewProgress(4*1024*1024+1);
    test_qbox_up_findnextblock(putRet,4*1024*1024+1,prog);
    CU_ASSERT_EQUAL(temp_index,2);
    CU_ASSERT_EQUAL(*(temp+0),0);
    CU_ASSERT_EQUAL(*(temp+1),1);
    //test branch: progs[i].ctx="end" progs[i].ctx!=null
    temp_index=0;
    prog = QBox_UP_NewProgress(4*1024*1024+1);
    prog->progs[0].ctx="end";
    test_qbox_up_findnextblock(putRet,4*1024*1024+1,prog);
    CU_ASSERT_EQUAL(temp_index,1);
    CU_ASSERT_EQUAL(*(temp+0),1);
    //test branch: i>=prog->blockCount
    temp_index=0;
    prog = QBox_UP_NewProgress(4*1024*1024);
    prog->progs[0].ctx="end";
    test_qbox_up_findnextblock(putRet,4*1024*1024,prog);
    CU_ASSERT_EQUAL(temp_index,0);
    //test breach: i > prog->blockCount progs[i].ctx!="end"
    temp_index=0;
    prog = QBox_UP_NewProgress(4*1024*1024+1);
    prog->progs[0].ctx="xxx";
    test_qbox_up_findnextblock(putRet,4*1024*1024+1,prog);
    CU_ASSERT_EQUAL(temp_index,2);
    CU_ASSERT_EQUAL(*(temp+0),0);
    CU_ASSERT_EQUAL(*(temp+1),1);
}

int block_notify_up(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    QBox_UP_Progress* prog = (QBox_UP_Progress*) self;
    if (blockIdx == prog->blockCount)
        return 0;
    return 1;
}

int block_notify_err299_up(void* self, int blockIdx, QBox_UP_Checksum* checksum)
{
    return 0;
}

void test_QBox_UP_Put(){
    QBox_UP_PutRet* putRet=malloc(sizeof(QBox_UP_PutRet));
    const char* fl=TESTFILE;
    QBox_ReaderAt f;
    QBox_Int64 fsize=29;
    QBox_UP_Progress* prog = NULL;
    //test branch: blockNotify=NULL
    f = QBox_FileReaderAt_Open(fl);
    prog = QBox_UP_NewProgress(29);
    err = QBox_UP_Put(&client,putRet, f, fsize, prog, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(err.code,200);

    //test branch: blockNotify!=NULL
    f = QBox_FileReaderAt_Open(fl);
    prog = QBox_UP_NewProgress(29);
    err = QBox_UP_Put(&client,putRet, f, fsize, prog, block_notify_up, NULL, &prog);
    CU_ASSERT_EQUAL(err.code,200);

    //test branch: keepGoing == 0 && (blkIndex + 1) >= prog->blockCount)
    f = QBox_FileReaderAt_Open(fl);
    prog = QBox_UP_NewProgress(29);
    err = QBox_UP_Put(&client,putRet, f, fsize, prog, block_notify_err299_up, NULL, &prog);
    CU_ASSERT_EQUAL(err.code,200);

    //test Error
    //test err299
    //test branch: keepGoing == 0 && (blkIndex + 1) < prog->blockCount)
    f = QBox_FileReaderAt_Open(TESTFILE_16M);
    prog = QBox_UP_NewProgress(4*1024*1024*2);
    err = QBox_UP_Put(&client,putRet, f, fsize, prog, block_notify_err299_up, NULL, &prog);
    CU_ASSERT_EQUAL(err.code,299);
    //test err401
    QBOX_ACCESS_KEY = "err";
    f = QBox_FileReaderAt_Open(fl);
    prog = QBox_UP_NewProgress(29);
    err = QBox_UP_Put(&client,putRet, f, fsize, prog, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(err.code,401);
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
}

/**//*---- test suites ------------------*/
int suite_init_up(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);
    return 0;
}

int suite_clean_up(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
    return 0;
}

QBOX_TESTS_BEGIN(up)
QBOX_TEST(test_QBox_UP_Mkblock)
QBOX_TEST(test_QBox_UP_NewProgress)
QBOX_TEST(test_QBox_UP_Progress_Release)
QBOX_TEST(test_QBox_UP_ResumableBlockput)
QBOX_TEST(test_QBox_UP_Mkfile)
QBOX_TEST(test_QBox_UP_FindNextBlock)
QBOX_TEST(test_QBox_UP_Put)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(up,suite_init_up,suite_clean_up)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsUp(int mode)
{
    myMode=mode;
    QBOX_TEST_REGISTE(up)
}
