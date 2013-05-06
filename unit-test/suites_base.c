/*
 ============================================================================
 Name        : suites_base.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "test.h"


QBox_Error err;
QBox_Client client;

//#define TESTFILE "/home/wsy/文档/c-sdk/unit-test/test_file.txt"
#define TESTFILE "test_file.txt"
void test_QBox_Seconds(){
    QBox_Int64 sec1,sec2;
    sec1=time(NULL);
    sec2=QBox_Seconds();
    CU_ASSERT(sec1<=sec2 && sec1>sec2-5);
}

void test_QBox_QueryEscape(){
    QBox_Bool fesc;
    char *str;
    //test branch:normal
    //(('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')
    //(spaceCount == 0 && hexCount == 0) {
    str=QBox_QueryEscape("abxzABXZ0189",&fesc);
    CU_ASSERT_EQUAL(fesc,QBox_False);
    CU_ASSERT_STRING_EQUAL(str,"abxzABXZ0189")
    //test branch:spaceCount > 0
    str=QBox_QueryEscape("abxz ABXZ 0189",&fesc);
    CU_ASSERT_EQUAL(fesc,QBox_True);
    CU_ASSERT_STRING_EQUAL(str,"abxz+ABXZ+0189");
    //test branch:hexCount > 0) {
    str=QBox_QueryEscape("abxz#ABXZ=0189",&fesc);
    CU_ASSERT_EQUAL(fesc,QBox_True);
    CU_ASSERT_STRING_EQUAL(str,"abxz%23ABXZ%3D0189")
    //test branch:asc(~)=126  c>'Z' c>'z' c>'9'
    //test branch:asc(-)=45   c<'A' c<'a' c<'0'
    str=QBox_QueryEscape("abxz-ABXZ~0189",&fesc);
    CU_ASSERT_EQUAL(fesc,QBox_False);
    CU_ASSERT_STRING_EQUAL(str,"abxz-ABXZ~0189")
}

void test_QBox_String_Concat2(){
    CU_ASSERT_STRING_EQUAL(QBox_String_Concat2("asdf","qwe"),"asdfqwe");
}

void test_QBox_String_Concat3(){
    CU_ASSERT_STRING_EQUAL(QBox_String_Concat3("as","df","qwe"),"asdfqwe");
}
void test_QBox_String_Concat(){
    CU_ASSERT_EQUAL(strncmp(QBox_String_Concat("a","s","d","f","q","w","e"),"asdfqwe",7),0);
    //CU_ASSERT_STRING_EQUAL(QBox_String_Concat("a","s","d","f","q","w","e"),"asdfqwe");
}
void test_QBox_String_Encode(){
    const char* s1="abcdABCD1234=+";
    const size_t cb = strlen(s1);
    const size_t cbDest = urlsafe_b64_encode(s1, cb, NULL, 0);
    char* dest = (char*)malloc(cbDest + 1);
    const size_t cbReal = urlsafe_b64_encode(s1, cb, dest, cbDest);
    dest[cbReal] = '\0';
    CU_ASSERT_STRING_EQUAL(QBox_String_Encode(s1),dest);
}

void test_QBox_Memory_Encode(){
    const char* s1="abcdABCD1234=+";
    const size_t cb = strlen(s1);
    CU_ASSERT_STRING_EQUAL(QBox_Memory_Encode(s1,cb),QBox_String_Encode(s1));
}

void test_QBox_String_Decode(){
    CU_ASSERT_STRING_EQUAL(QBox_String_Decode(QBox_String_Encode("abcdABCD1234=+-*")),"abcdABCD1234=+-*");
}

void test_QBox_Buffer_Init(){
    size_t size=16;
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buff,size);
    CU_ASSERT_EQUAL(buff->curr,buff->buf);
    CU_ASSERT_EQUAL(buff->bufEnd,buff->buf+size);
}

void test_QBox_Buffer_Reset(){
    size_t size=16;
    char* s="1234567890";
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buff,size);
    QBox_Buffer_Write(buff,s,1);
    CU_ASSERT_NOT_EQUAL(buff->curr,buff->buf);
    QBox_Buffer_Reset(buff);
    CU_ASSERT_EQUAL(buff->curr,buff->buf);
}

void test_QBox_Buffer_Cleanup(){
    size_t size=16;
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buff,size);
    CU_ASSERT_NOT_EQUAL(buff->buf,NULL);
    QBox_Buffer_Cleanup(buff);
    CU_ASSERT_EQUAL(buff->buf,NULL);
    QBox_Buffer_Cleanup(buff);
    CU_ASSERT_EQUAL(buff->buf,NULL);
}

void test_QBox_Buffer_Len(){
    size_t size=16;
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buff,size);
    CU_ASSERT_EQUAL(QBox_Buffer_Len(buff),0);
    QBox_Buffer_Write(buff,"12345",5);
    CU_ASSERT_EQUAL(QBox_Buffer_Len(buff),5);
}

void test_QBox_Buffer_CStr(){
    size_t size=2;
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    //test branch self->curr >= self->bufEnd
    QBox_Buffer_Init(buff,size);
    CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(buff),"");
    QBox_Buffer_Write(buff,"12345",2);
    CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(buff),"12");
    //test branch self->curr <self->bufEnd
    QBox_Buffer_Init(buff,size);
    QBox_Buffer_Write(buff,"12345",5);
    CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(buff),"12345");
}

void test_QBox_Buffer_Write(){
    size_t size=4;
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    //test branch self->curr + n <self->bufEnd
    QBox_Buffer_Init(buff,size);
    QBox_Buffer_Write(buff,"12345",1);
    CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(buff),"1");
    //test branch self->curr + n =self->bufEnd
    QBox_Buffer_Init(buff,size);
    QBox_Buffer_Write(buff,"12345",2);
    CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(buff),"12");
    //test branch self->curr + n > self->bufEnd
    QBox_Buffer_Init(buff,size);
    QBox_Buffer_Write(buff,"12345",5);
    CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(buff),"12345");
}

void test_QBox_Buffer_Fwrite(){
    size_t size=4;
    QBox_Buffer* buff=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buff,size);
    QBox_Buffer_Fwrite("12345",1,5,buff);
    CU_ASSERT_STRING_EQUAL(QBox_Buffer_CStr(buff),"12345");
}

void test_QBox_Null_Fwrite(){
    CU_ASSERT_EQUAL(QBox_Null_Fwrite(NULL,1,5,NULL),5);
}

void test_QBox_FILE_Reader(){
    FILE* file;
    QBox_Reader reader;
    file=fopen(TESTFILE,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    reader=QBox_FILE_Reader(file);
    CU_ASSERT_EQUAL(reader.self,file);
    reader=QBox_FILE_Reader(NULL);
    CU_ASSERT_EQUAL(reader.self,NULL);
}

void test_QBox_BufReader_Read(){
    int len;
    void *buf=malloc(64);
    QBox_BufReader* bufReader=malloc(sizeof(QBox_BufReader));
    QBox_Reader reader;
    reader=QBox_Buffer_Reader(bufReader,"test_QBox_BufReader_Read",25);
    //test branch:n<max
	len=reader.Read(buf,0,9,reader.self);
	*((char*)buf+9)='\0';
	CU_ASSERT_EQUAL(len,9);
	CU_ASSERT_EQUAL(bufReader->off,9);
	CU_ASSERT_STRING_EQUAL(buf,"test_QBox");
    //test branch:n>max
	len=reader.Read(buf,0,30,reader.self);
	CU_ASSERT_EQUAL(len,16);
	CU_ASSERT_EQUAL(bufReader->off,25);
	CU_ASSERT_STRING_EQUAL(buf,"_BufReader_Read");
	//test branch:max<=0
	len=reader.Read(buf,0,0,reader.self);
	CU_ASSERT_EQUAL(len,0);
	len=reader.Read(buf,0,-1,reader.self);
	CU_ASSERT_EQUAL(len,0);
}

void test_QBox_buffer_Reader(){
    QBox_BufReader* bufReader=malloc(sizeof(QBox_BufReader));
    QBox_Reader reader;
    reader=QBox_Buffer_Reader(bufReader,"test_QBox_buffer_Reader",23);
    CU_ASSERT_EQUAL(reader.self,bufReader);
    CU_ASSERT_STRING_EQUAL(((QBox_BufReader*)(reader.self))->buf,"test_QBox_buffer_Reader");
    CU_ASSERT_EQUAL(((QBox_BufReader*)(reader.self))->off,0);
    CU_ASSERT_EQUAL(((QBox_BufReader*)(reader.self))->limit,23);
}

void test_QBox_sectionReader_Read(){
    char *buff=malloc(256);
    QBox_Reader reader;
    QBox_ReaderAt readerAt;
    int result=-1;
    //test branch:n<max
    readerAt=QBox_FileReaderAt_Open(TESTFILE);
    reader=QBox_SectionReader(readerAt,0,28);
    result=reader.Read(buff,0,9,reader.self);
    buff[9]='\0';
    CU_ASSERT_EQUAL(result,9);
    CU_ASSERT_STRING_EQUAL(buff,"JUST FOR ");
    result=reader.Read(buff,0,30,reader.self);
    buff[19]='\0';
    CU_ASSERT_EQUAL(result,19);
    CU_ASSERT_STRING_EQUAL(buff,"TEST!JUST FOR TEST!");
    //test branch:n>max
    result=reader.Read(buff,0,0,reader.self);
    CU_ASSERT_EQUAL(result,0);
    result=reader.Read(buff,0,-5,reader.self);
    CU_ASSERT_EQUAL(result,0);
}

void test_QBox_SectionReader_Release(){
    QBox_Reader reader;
    QBox_ReaderAt readerAt;

    readerAt=QBox_FileReaderAt_Open(TESTFILE);
    reader=QBox_SectionReader(readerAt,0,29);

    QBox_SectionReader_Release(reader.self);
}

void test_QBox_FileReaderAt_Open(){
    CU_ASSERT_EQUAL(QBox_FileReaderAt_Open("bazingo").self,NULL);
}

void test_QBox_FileReaderAt_Close(){
    QBox_FileReaderAt_Close(TESTFILE);
}


/**//*---- test suites ------------------*/
int suite_init_base(void)
{
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_base(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(testcases_base)
//*base.c
QBOX_TEST(test_QBox_Seconds)
QBOX_TEST(test_QBox_QueryEscape)
QBOX_TEST(test_QBox_String_Concat2)
QBOX_TEST(test_QBox_String_Concat3)
QBOX_TEST(test_QBox_String_Concat)
QBOX_TEST(test_QBox_String_Encode)
QBOX_TEST(test_QBox_Memory_Encode)
QBOX_TEST(test_QBox_String_Decode)
QBOX_TEST(test_QBox_Buffer_Init)
QBOX_TEST(test_QBox_Buffer_Reset)
QBOX_TEST(test_QBox_Buffer_Cleanup)
QBOX_TEST(test_QBox_Buffer_Len)
QBOX_TEST(test_QBox_Buffer_CStr)
QBOX_TEST(test_QBox_Buffer_Write)
QBOX_TEST(test_QBox_Buffer_Fwrite)
QBOX_TEST(test_QBox_Null_Fwrite)
QBOX_TEST(test_QBox_FILE_Reader)
//base.c*/
//*base_io.c
QBOX_TEST(test_QBox_BufReader_Read)
QBOX_TEST(test_QBox_buffer_Reader)
QBOX_TEST(test_QBox_sectionReader_Read)
QBOX_TEST(test_QBox_FileReaderAt_Open)
QBOX_TEST(test_QBox_SectionReader_Release)
QBOX_TEST(test_QBox_FileReaderAt_Close)
//base_io.c*/
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(testcases_base,suite_init_base,suite_clean_base)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsBase(void)
{
        QBOX_TEST_REGISTE(base)
}

