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
#include "../qiniu/base.h"
#include "../qiniu/rs.h"
#include "test.h"


Qiniu_Error err;
Qiniu_Client client;

//#define TESTFILE "/home/wsy/文档/c-sdk/unit-test/test_file.txt"
#define TESTFILE "test_file_1M.txt"

void test_Qiniu_Seconds(){
    Qiniu_Int64 sec1,sec2;
    sec1=time(NULL);
    sec2=Qiniu_Seconds();
    CU_ASSERT(sec1<=sec2 && sec1>sec2-5);
}

void test_Qiniu_QueryEscape(){
    Qiniu_Bool fesc;
    char *str;
    //test branch:normal
    //(('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')
    //(spaceCount == 0 && hexCount == 0) {
    str=Qiniu_QueryEscape("abxzABXZ0189",&fesc);
    CU_ASSERT_EQUAL(fesc,Qiniu_False);
    CU_ASSERT_STRING_EQUAL(str,"abxzABXZ0189")
    //test branch:spaceCount > 0
    str=Qiniu_QueryEscape("abxz ABXZ 0189",&fesc);
    CU_ASSERT_EQUAL(fesc,Qiniu_True);
    CU_ASSERT_STRING_EQUAL(str,"abxz+ABXZ+0189");
    //test branch:hexCount > 0) {
    str=Qiniu_QueryEscape("abxz#ABXZ=0189",&fesc);
    CU_ASSERT_EQUAL(fesc,Qiniu_True);
    CU_ASSERT_STRING_EQUAL(str,"abxz%23ABXZ%3D0189")
    //test branch:asc(~)=126  c>'Z' c>'z' c>'9'
    //test branch:asc(-)=45   c<'A' c<'a' c<'0'
    str=Qiniu_QueryEscape("abxz-ABXZ~0189",&fesc);
    CU_ASSERT_EQUAL(fesc,Qiniu_False);
    CU_ASSERT_STRING_EQUAL(str,"abxz-ABXZ~0189")
}

void test_Qiniu_String_Concat2(){
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Concat2("asdf","qwe"),"asdfqwe");
}

void test_Qiniu_String_Concat3(){
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Concat3("as","df","qwe"),"asdfqwe");
}
void test_Qiniu_String_Concat(){
    CU_ASSERT_EQUAL(strncmp(Qiniu_String_Concat("a","s","d","f","q","w","e"),"asdfqwe",7),0);
    //CU_ASSERT_STRING_EQUAL(Qiniu_String_Concat("a","s","d","f","q","w","e"),"asdfqwe");
}
void test_Qiniu_String_Encode(){
    const char* s1="abcdABCD1234=+";
    const size_t cb = strlen(s1);
    const size_t cbDest = urlsafe_b64_encode(s1, cb, NULL, 0);
    char* dest = (char*)malloc(cbDest + 1);
    const size_t cbReal = urlsafe_b64_encode(s1, cb, dest, cbDest);
    dest[cbReal] = '\0';
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Encode(s1),dest);
}

void test_Qiniu_Memory_Encode(){
    const char* s1="abcdABCD1234=+";
    const size_t cb = strlen(s1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Memory_Encode(s1,cb),Qiniu_String_Encode(s1));
}

void test_Qiniu_String_Decode(){
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Decode(Qiniu_String_Encode("abcdABCD1234=+-*")),"abcdABCD1234=+-*");
}

void test_Qiniu_Buffer_Init(){
    size_t size=16;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    CU_ASSERT_EQUAL(buff->curr,buff->buf);
    CU_ASSERT_EQUAL(buff->bufEnd,buff->buf+size);
}

void test_Qiniu_Buffer_Reset(){
    size_t size=16;
    char* s="1234567890";
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Write(buff,s,1);
    CU_ASSERT_NOT_EQUAL(buff->curr,buff->buf);
    Qiniu_Buffer_Reset(buff);
    CU_ASSERT_EQUAL(buff->curr,buff->buf);
}

void test_Qiniu_Buffer_Cleanup(){
    size_t size=16;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    CU_ASSERT_NOT_EQUAL(buff->buf,NULL);
    Qiniu_Buffer_Cleanup(buff);
    CU_ASSERT_EQUAL(buff->buf,NULL);
    Qiniu_Buffer_Cleanup(buff);
    CU_ASSERT_EQUAL(buff->buf,NULL);
}

void test_Qiniu_Buffer_Len(){
    size_t size=16;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    CU_ASSERT_EQUAL(Qiniu_Buffer_Len(buff),0);
    Qiniu_Buffer_Write(buff,"Qiniu",5);
    CU_ASSERT_EQUAL(Qiniu_Buffer_Len(buff),5);
}

void test_Qiniu_Buffer_CStr(){
    size_t size=2;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    //test branch self->curr >= self->bufEnd
    Qiniu_Buffer_Init(buff,size);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"");
    Qiniu_Buffer_Write(buff,"Qiniu",2);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qi");
    //test branch self->curr <self->bufEnd
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Write(buff,"Qiniu",5);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu");
}

void test_Qiniu_Buffer_PutChar(){
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,1);
    Qiniu_Buffer_PutChar(buff,'Q');
    Qiniu_Buffer_PutChar(buff,'i');
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qi");
}

void test_Qiniu_Buffer_Write(){
    size_t size=4;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    //test branch self->curr + n <self->bufEnd
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Write(buff,"Qiniu",1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Q");
    //test branch self->curr + n =self->bufEnd
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Write(buff,"Qiniu",2);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qi");
    //test branch self->curr + n > self->bufEnd
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Write(buff,"Qiniu",5);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu");
}

void test_Qiniu_Buffer_Fwrite(){
    size_t size=4;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Fwrite("Qiniu",1,5,buff);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu");
}
void test_Qiniu_BufWriter(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Writer writer;
    Qiniu_Buffer_Init(buff,8);
    writer=Qiniu_BufWriter(buff);
    writer.Write("Qiniu",1,5,writer.self);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(writer.self),"Qiniu");
}

void test_Qiniu_Buffer_Expand(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,8);
    char* p;
    CU_ASSERT_EQUAL(buff->bufEnd-buff->buf,8);
    p=Qiniu_Buffer_Expand(buff,3);
    CU_ASSERT_EQUAL(buff->bufEnd-buff->buf,8);
    CU_ASSERT_EQUAL(p,buff->buf);
    p=Qiniu_Buffer_Expand(buff,10);
    CU_ASSERT_EQUAL(buff->bufEnd-buff->buf,16);
    CU_ASSERT_EQUAL(p,buff->buf);
}

void test_Qiniu_Buffer_Commit(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,8);
    char* p=buff->buf+1;
    Qiniu_Buffer_Commit(buff,p);
    CU_ASSERT_EQUAL(buff->curr,p);
}
void test_Qiniu_Buffer_AppendUint(){
    Qiniu_Uint64 v;
    Qiniu_Buffer *buff;
    buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    v=-1;
    Qiniu_Buffer_AppendUint(buff,v);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"18446744073709551615");
    v=0;
    Qiniu_Buffer_Init(buff,32);
    Qiniu_Buffer_AppendUint(buff,v);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"0");
    v=(Qiniu_Uint64)1<<31;
    Qiniu_Buffer_Init(buff,32);
    Qiniu_Buffer_AppendUint(buff,v);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"2147483648");
}

void test_Qiniu_Buffer_AppendInt(){
    Qiniu_Int64 v;
    Qiniu_Buffer *buff;
    buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    v=-1;
    Qiniu_Buffer_AppendInt(buff,v);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"-1");
    v=0;
    Qiniu_Buffer_Init(buff,32);
    Qiniu_Buffer_AppendInt(buff,v);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"0");
    v=((Qiniu_Int64)1<<63);
    Qiniu_Buffer_Init(buff,32);
    Qiniu_Buffer_AppendInt(buff,v);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"-9223372036854775808");
    v=~((Qiniu_Int64)1<<63);
    Qiniu_Buffer_Init(buff,32);
    Qiniu_Buffer_AppendInt(buff,v);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"9223372036854775807");
}

void test_Qiniu_Buffer_AppendError(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Error error;
    error.code=200;
    error.message=NULL;
    Qiniu_Buffer_Init(buff,16);
    Qiniu_Buffer_AppendError(buff,error);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"E200");
    error.code=200;
    error.message=malloc(2);
    strcpy(error.message,"ok");
    Qiniu_Buffer_Init(buff,16);
    Qiniu_Buffer_AppendError(buff,error);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"E200 ok");
}

void test_Qiniu_Buffer_AppendEncodedBinary(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    const char* str="qiniu";
    size_t cb=5;
    Qiniu_Buffer_Init(buff,16);
    Qiniu_Buffer_AppendEncodedBinary(buff,str,cb);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),Qiniu_String_Encode(str));
}

void test_qiniu_buffer_appendformatv(Qiniu_Buffer* self, const char* fmt, ...){
    Qiniu_Valist args;
    va_start(args.items, fmt);
    Qiniu_Buffer_AppendFormatV(self, fmt, &args);
}
void test_Qiniu_Buffer_appendUint(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %u",-1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 4294967295");
}
void test_Qiniu_Buffer_appendInt(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %d",64);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 64");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %d",-1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu -1");
}
void test_Qiniu_Buffer_appendUint64(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %U",((Qiniu_Uint64)1<<63));
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 9223372036854775808");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %U",(Qiniu_Uint64)-1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 18446744073709551615");
}
void test_Qiniu_Buffer_appendInt64(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %D",~((Qiniu_Int64)1<<63));
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 9223372036854775807");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %D",(Qiniu_Int64)1<<63);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu -9223372036854775808");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %D",(Qiniu_Int64)-1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu -1");
}
void test_Qiniu_Buffer_appendString(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %s","c-sdk");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu c-sdk");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %s",NULL);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu (null)");
}
void test_Qiniu_Buffer_appendEncodedString(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %S","c-sdk");
    char *str=malloc(64);
    strcpy(str,"Qiniu ");
    strcat(str,Qiniu_String_Encode("c-sdk"));
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),str);
}
void test_Qiniu_Buffer_appendError(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Error error;
    error.code=200;
    error.message="ok";
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %E",error);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu E200 ok");
}
void test_Qiniu_Buffer_appendPercent(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %%");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu %");
}
void test_Qiniu_Buffer_AppendFormatV(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
}


void test_Qiniu_Null_Fwrite(){
    CU_ASSERT_EQUAL(Qiniu_Null_Fwrite(NULL,1,5,NULL),5);
}

void test_Qiniu_FILE_Reader(){
    FILE* file;
    Qiniu_Reader reader;
    file=fopen(TESTFILE,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    reader=Qiniu_FILE_Reader(file);
    CU_ASSERT_EQUAL(reader.self,file);
    reader=Qiniu_FILE_Reader(NULL);
    CU_ASSERT_EQUAL(reader.self,NULL);
}


/*
void test_Qiniu_BufReader_Read(){
    int len;
    void *buf=malloc(64);
    Qiniu_BufReader* bufReader=malloc(sizeof(Qiniu_BufReader));
    Qiniu_Reader reader;
    reader=Qiniu_Buffer_Reader(bufReader,"test_Qiniu_BufReader_Read",25);
    //test branch:n<max
	len=reader.Read(buf,0,9,reader.self);
	*((char*)buf+9)='\0';
	CU_ASSERT_EQUAL(len,9);
	CU_ASSERT_EQUAL(bufReader->off,9);
	CU_ASSERT_STRING_EQUAL(buf,"test_Qiniu");
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

void test_Qiniu_buffer_Reader(){
    Qiniu_BufReader* bufReader=malloc(sizeof(Qiniu_BufReader));
    Qiniu_Reader reader;
    reader=Qiniu_Buffer_Reader(bufReader,"test_Qiniu_buffer_Reader",23);
    CU_ASSERT_EQUAL(reader.self,bufReader);
    CU_ASSERT_STRING_EQUAL(((Qiniu_BufReader*)(reader.self))->buf,"test_Qiniu_buffer_Reader");
    CU_ASSERT_EQUAL(((Qiniu_BufReader*)(reader.self))->off,0);
    CU_ASSERT_EQUAL(((Qiniu_BufReader*)(reader.self))->limit,23);
}

void test_Qiniu_sectionReader_Read(){
    char *buff=malloc(256);
    Qiniu_Reader reader;
    Qiniu_ReaderAt readerAt;
    int result=-1;
    //test branch:n<max
    readerAt=Qiniu_FileReaderAt_Open(TESTFILE);
    reader=Qiniu_SectionReader(readerAt,0,28);
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

void test_Qiniu_SectionReader_Release(){
    Qiniu_Reader reader;
    Qiniu_ReaderAt readerAt;

    readerAt=Qiniu_FileReaderAt_Open(TESTFILE);
    reader=Qiniu_SectionReader(readerAt,0,29);

    Qiniu_SectionReader_Release(reader.self);
}

void test_Qiniu_FileReaderAt_Open(){
    CU_ASSERT_EQUAL(Qiniu_FileReaderAt_Open("bazingo").self,NULL);
}

void test_Qiniu_FileReaderAt_Close(){
    Qiniu_FileReaderAt_Close(TESTFILE);
}
*/

/**//*---- test suites ------------------*/
int suite_init_base(void)
{
	QINIU_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QINIU_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	Qiniu_Zero(client);
	Qiniu_Global_Init(-1);

    Qiniu_Client_InitNoAuth(&client,1024);
	//Qiniu_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_base(void)
{
	Qiniu_Client_Cleanup(&client);
	Qiniu_Global_Cleanup();

    return 0;
}

QINIU_TESTS_BEGIN(testcases_base)
//*base.c
QINIU_TEST(test_Qiniu_Seconds)
QINIU_TEST(test_Qiniu_QueryEscape)
QINIU_TEST(test_Qiniu_String_Concat2)
QINIU_TEST(test_Qiniu_String_Concat3)
QINIU_TEST(test_Qiniu_String_Concat)
QINIU_TEST(test_Qiniu_String_Encode)
QINIU_TEST(test_Qiniu_Memory_Encode)
QINIU_TEST(test_Qiniu_String_Decode)
QINIU_TEST(test_Qiniu_Buffer_Init)
QINIU_TEST(test_Qiniu_Buffer_Reset)
QINIU_TEST(test_Qiniu_Buffer_Cleanup)
QINIU_TEST(test_Qiniu_Buffer_Len)
QINIU_TEST(test_Qiniu_Buffer_CStr)
QINIU_TEST(test_Qiniu_Buffer_PutChar)
QINIU_TEST(test_Qiniu_Buffer_Write)
QINIU_TEST(test_Qiniu_Buffer_Fwrite)
QINIU_TEST(test_Qiniu_BufWriter)
QINIU_TEST(test_Qiniu_Buffer_Expand)
QINIU_TEST(test_Qiniu_Buffer_Commit)
QINIU_TEST(test_Qiniu_Buffer_AppendUint)
QINIU_TEST(test_Qiniu_Buffer_AppendInt)
QINIU_TEST(test_Qiniu_Buffer_AppendError)
QINIU_TEST(test_Qiniu_Buffer_AppendEncodedBinary)
QINIU_TEST(test_Qiniu_Buffer_appendUint)
QINIU_TEST(test_Qiniu_Buffer_appendInt)
QINIU_TEST(test_Qiniu_Buffer_appendUint64)
QINIU_TEST(test_Qiniu_Buffer_appendInt64)
QINIU_TEST(test_Qiniu_Buffer_appendString)
QINIU_TEST(test_Qiniu_Buffer_appendEncodedString)
QINIU_TEST(test_Qiniu_Buffer_appendError)
QINIU_TEST(test_Qiniu_Buffer_appendPercent)
QINIU_TEST(test_Qiniu_Buffer_AppendFormatV)
QINIU_TEST(test_Qiniu_Null_Fwrite)
QINIU_TEST(test_Qiniu_FILE_Reader)
//base.c*/
/*base_io.c
QINIU_TEST(test_Qiniu_BufReader_Read)
QINIU_TEST(test_Qiniu_buffer_Reader)
QINIU_TEST(test_Qiniu_sectionReader_Read)
QINIU_TEST(test_Qiniu_FileReaderAt_Open)
QINIU_TEST(test_Qiniu_SectionReader_Release)
QINIU_TEST(test_Qiniu_FileReaderAt_Close)
//base_io.c*/
QINIU_TESTS_END()

QINIU_SUITES_BEGIN()
QINIU_SUITE_EX(testcases_base,suite_init_base,suite_clean_base)
QINIU_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsBase(void)
{
        QINIU_TEST_REGISTE(base)
}

