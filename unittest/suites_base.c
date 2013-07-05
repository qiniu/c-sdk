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
#include "../qiniu/conf.h"
#include "test.h"


Qiniu_Error err;

//#define TESTFILE "/home/wsy/文档/c-sdk/unit-test/test_file.txt"
#define TESTFILE_I "test_file_64b.txt"
#define TESTFILE_O "test_file_output.txt"

static void test_Qiniu_Seconds(){
    Qiniu_Int64 sec1,sec2;
    sec1=time(NULL);
    sec2=Qiniu_Seconds();
    CU_ASSERT(sec1<=sec2 && sec1>sec2-5);
}

static void test_Qiniu_QueryEscape(){
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

static void test_Qiniu_String_Concat2(){
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Concat2("asdf","qwe"),"asdfqwe");
}

static void test_Qiniu_String_Concat3(){
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Concat3("as","df","qwe"),"asdfqwe");
}
static void test_Qiniu_String_Concat(){
    CU_ASSERT_EQUAL(strncmp(Qiniu_String_Concat("a","s","d","f","q","w","e"),"asdfqwe",7),0);
    //CU_ASSERT_STRING_EQUAL(Qiniu_String_Concat("a","s","d","f","q","w","e"),"asdfqwe");
}
static void test_Qiniu_String_Encode(){
    const char* s1="abcdABCD1234=+";
    const size_t cb = strlen(s1);
    const size_t cbDest = urlsafe_b64_encode(s1, cb, NULL, 0);
    char* dest = (char*)malloc(cbDest + 1);
    const size_t cbReal = urlsafe_b64_encode(s1, cb, dest, cbDest);
    dest[cbReal] = '\0';
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Encode(s1),dest);
}

static void test_Qiniu_Memory_Encode(){
    const char* s1="abcdABCD1234=+";
    const size_t cb = strlen(s1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Memory_Encode(s1,cb),Qiniu_String_Encode(s1));
}

static void test_Qiniu_String_Decode(){
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Decode(Qiniu_String_Encode("abcdABCD1234=+-*")),"abcdABCD1234=+-*");
}

static void test_Qiniu_Buffer_Init(){
    size_t size=16;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    CU_ASSERT_EQUAL(buff->curr,buff->buf);
    CU_ASSERT_EQUAL(buff->bufEnd,buff->buf+size);
}

static void test_Qiniu_Buffer_Reset(){
    size_t size=16;
    char* s="1234567890";
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Write(buff,s,1);
    CU_ASSERT_NOT_EQUAL(buff->curr,buff->buf);
    Qiniu_Buffer_Reset(buff);
    CU_ASSERT_EQUAL(buff->curr,buff->buf);
}

static void test_Qiniu_Buffer_Cleanup(){
    size_t size=16;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    CU_ASSERT_NOT_EQUAL(buff->buf,NULL);
    Qiniu_Buffer_Cleanup(buff);
    CU_ASSERT_EQUAL(buff->buf,NULL);
    Qiniu_Buffer_Cleanup(buff);
    CU_ASSERT_EQUAL(buff->buf,NULL);
}

static void test_Qiniu_Buffer_Len(){
    size_t size=16;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    CU_ASSERT_EQUAL(Qiniu_Buffer_Len(buff),0);
    Qiniu_Buffer_Write(buff,"Qiniu",5);
    CU_ASSERT_EQUAL(Qiniu_Buffer_Len(buff),5);
}

static void test_Qiniu_Buffer_CStr(){
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

static void test_Qiniu_Buffer_PutChar(){
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,1);
    Qiniu_Buffer_PutChar(buff,'Q');
    Qiniu_Buffer_PutChar(buff,'i');
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qi");
}

static void test_Qiniu_Buffer_Write(){
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

static void test_Qiniu_Buffer_Fwrite(){
    size_t size=4;
    Qiniu_Buffer* buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,size);
    Qiniu_Buffer_Fwrite("Qiniu",1,5,buff);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu");
}
static void test_Qiniu_BufWriter(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Writer writer;
    Qiniu_Buffer_Init(buff,8);
    writer=Qiniu_BufWriter(buff);
    writer.Write("Qiniu",1,5,writer.self);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(writer.self),"Qiniu");
}

static void test_Qiniu_Buffer_Expand(){
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

static void test_Qiniu_Buffer_Commit(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,8);
    char* p=buff->buf+1;
    Qiniu_Buffer_Commit(buff,p);
    CU_ASSERT_EQUAL(buff->curr,p);
}
static void test_Qiniu_Buffer_AppendUint(){
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

static void test_Qiniu_Buffer_AppendInt(){
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

static void test_Qiniu_Buffer_AppendError(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Error error;
    error.code=200;
    error.message=NULL;
    Qiniu_Buffer_Init(buff,16);
    Qiniu_Buffer_AppendError(buff,error);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"E200");
    error.code=200;
    error.message=malloc(3);
    strcpy(error.message,"ok");
    Qiniu_Buffer_Init(buff,16);
    Qiniu_Buffer_AppendError(buff,error);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"E200 ok");
}

static void test_Qiniu_Buffer_AppendEncodedBinary(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    const char* str="qiniu";
    size_t cb=5;
    Qiniu_Buffer_Init(buff,16);
    Qiniu_Buffer_AppendEncodedBinary(buff,str,cb);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),Qiniu_String_Encode(str));
}

static void test_qiniu_buffer_appendformatv(Qiniu_Buffer* self, const char* fmt, ...){
    Qiniu_Valist args;
    va_start(args.items, fmt);
    Qiniu_Buffer_AppendFormatV(self, fmt, &args);
}

static void test_Qiniu_Buffer_appendUint(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %u",-1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 4294967295");
}

static void test_Qiniu_Buffer_appendInt(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %d",64);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 64");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %d",-1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu -1");
}

static void test_Qiniu_Buffer_appendUint64(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %U",((Qiniu_Uint64)1<<63));
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 9223372036854775808");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %U",(Qiniu_Uint64)-1);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 18446744073709551615");
}

static void test_Qiniu_Buffer_appendInt64(){
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

static void test_Qiniu_Buffer_appendString(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %s","c-sdk");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu c-sdk");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %s",NULL);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu (null)");
}

static void test_Qiniu_Buffer_appendEncodedString(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %S","c-sdk");
    char *str=malloc(64);
    strcpy(str,"Qiniu ");
    strcat(str,Qiniu_String_Encode("c-sdk"));
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),str);
}

static void test_Qiniu_Buffer_appendError(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Error error;
    error.code=200;
    error.message="ok";
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %E",error);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu E200 ok");
}

static void test_Qiniu_Buffer_appendPercent(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %%");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu %");
}

static void test_Qiniu_FnAppender(Qiniu_Buffer* self, Qiniu_Valist* ap){
    Qiniu_Buffer_Write(self, "test", 4);
}

static void test_Qiniu_Format_Register(){
    Qiniu_Format_Register('t',test_Qiniu_FnAppender);
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %t");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu test");

    Qiniu_Format_Register('汉',test_Qiniu_FnAppender);
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %汉");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu %汉");
}

static void test_Qiniu_Buffer_AppendFormatV(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"%s%s %U","Qi","niu",777);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 777");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %汉字测试");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu %汉字测试");
    Qiniu_Buffer_Init(buff,32);
    test_qiniu_buffer_appendformatv(buff,"Qiniu %n");
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu %n");
}

static void test_Qiniu_Buffer_AppendFormat(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    Qiniu_Buffer_AppendFormat(buff,"%s%s %U","Qi","niu",777);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_CStr(buff),"Qiniu 777");
}

static void test_Qiniu_Buffer_Format(){
    Qiniu_Buffer *buff=malloc(sizeof(Qiniu_Buffer));
    Qiniu_Buffer_Init(buff,32);
    CU_ASSERT_STRING_EQUAL(Qiniu_Buffer_Format(buff,"%s%s %U","Qi","niu",777),"Qiniu 777");
}

static void test_Qiniu_String_Format(){
    CU_ASSERT_STRING_EQUAL(Qiniu_String_Format(32,"%s%s %U","Qi","niu",777),"Qiniu 777");
}

static void test_Qiniu_FILE_Reader(){
    FILE* file;
    Qiniu_Reader reader;
    file=fopen(TESTFILE_I,"r");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    reader=Qiniu_FILE_Reader(file);
    CU_ASSERT_EQUAL(reader.self,file);
    fclose(file);
    reader=Qiniu_FILE_Reader(NULL);
    CU_ASSERT_EQUAL(reader.self,NULL);
}

static void test_Qiniu_FILE_Writer(){
    FILE* file;
    Qiniu_Writer writer;
    file=fopen(TESTFILE_O,"w");
    if(file==NULL){CU_ASSERT_NOT_EQUAL(file,NULL);return;}
    writer=Qiniu_FILE_Writer(file);
    CU_ASSERT_EQUAL(writer.self,file);
    fclose(file);
    writer=Qiniu_FILE_Writer(NULL);
    CU_ASSERT_EQUAL(writer.self,NULL);
}

static void test_Qiniu_Copy(){
    Qiniu_Writer writer;
    Qiniu_Reader reader;

    FILE* readFile=fopen(TESTFILE_I,"r");
    if(readFile==NULL){CU_ASSERT_NOT_EQUAL(readFile,NULL);return;}
    reader=Qiniu_FILE_Reader(readFile);

    FILE* writeFile=fopen(TESTFILE_O,"w");
    if(writeFile==NULL){CU_ASSERT_NOT_EQUAL(writeFile,NULL);return;}
    writer=Qiniu_FILE_Writer(writeFile);

    err=Qiniu_Copy(writer,reader,NULL,16,NULL);
    fclose(readFile);
    fclose(writeFile);
    CU_ASSERT_EQUAL(err.code,200);
    char *str1,*str2;
    str1=malloc(128);
    str2=malloc(128);
    readFile=fopen(TESTFILE_I,"r");
    if(readFile==NULL){CU_ASSERT_NOT_EQUAL(readFile,NULL);return;}
    reader=Qiniu_FILE_Reader(readFile);
    reader.Read(str1, 1, 64, reader.self);
    fclose(readFile);
    readFile=fopen(TESTFILE_O,"r");
    if(readFile==NULL){CU_ASSERT_NOT_EQUAL(readFile,NULL);return;}
    reader=Qiniu_FILE_Reader(readFile);
    reader.Read(str2, 1, 64, reader.self);
    *(str1+64)='\0';
    *(str2+64)='\0';
    fclose(readFile);
    CU_ASSERT_STRING_EQUAL(str1,str2);
    free(str1);
    free(str2);

    readFile=fopen(TESTFILE_I,"r");
    if(readFile==NULL){CU_ASSERT_NOT_EQUAL(readFile,NULL);return;}
    reader=Qiniu_FILE_Reader(readFile);

    writeFile=fopen(TESTFILE_O,"w");
    if(writeFile==NULL){CU_ASSERT_NOT_EQUAL(writeFile,NULL);return;}
    writer=Qiniu_FILE_Writer(writeFile);

    char *buff=malloc(128);
    Qiniu_Int64 len=0;
    err=Qiniu_Copy(writer,reader,buff,128,&len);

    fclose(readFile);
    fclose(writeFile);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(len,64);
    str2=malloc(128);
    readFile=fopen(TESTFILE_O,"r");
    if(readFile==NULL){CU_ASSERT_NOT_EQUAL(readFile,NULL);return;}
    reader=Qiniu_FILE_Reader(readFile);
    reader.Read(str2, 1, len, reader.self);
    *(buff+len)='\0';
    *(str2+len)='\0';
    fclose(readFile);
    CU_ASSERT_STRING_EQUAL(buff,str2);
    free(buff);
    free(str2);
}

static void test_Qiniu_Null_Fwrite(){
    CU_ASSERT_EQUAL(Qiniu_Null_Fwrite(NULL,1,5,NULL),5);
}

static void test_Qiniu_Null_Log(){
    Qiniu_Null_Log("test");
}

static void test_qiniu_logv(Qiniu_Writer w, int ilvl, const char* fmt, ...){
    Qiniu_Valist args;
    va_start(args.items, fmt);
    Qiniu_Logv(w, ilvl, fmt, &args);
}

static void test_Qiniu_Logv(){
    Qiniu_Writer writer;
    Qiniu_Reader reader;
    FILE* writeFile;
    FILE* readFile;
    char *str;

    writeFile=fopen(TESTFILE_O,"w");
    if(writeFile==NULL){CU_ASSERT_NOT_EQUAL(writeFile,NULL);return;}
    writer=Qiniu_FILE_Writer(writeFile);
    test_qiniu_logv(writer,0,"Qiniu %sv%d","test",7);
    fclose(writeFile);

    str=malloc(128);
    readFile=fopen(TESTFILE_O,"r");
    if(readFile==NULL){CU_ASSERT_NOT_EQUAL(readFile,NULL);return;}
    reader=Qiniu_FILE_Reader(readFile);
    reader.Read(str, 1, 128, reader.self);
    *(str+22)='\0';
    fclose(readFile);
    CU_ASSERT_STRING_EQUAL(str,"[DEBUG] Qiniu testv7\n");
    free(str);

}

static void test_Qiniu_Stderr_Info(){
    Qiniu_Stderr_Info("Qiniu %sv%d","test",7);
}

static void test_Qiniu_Stderr_Warn(){
    Qiniu_Stderr_Warn("Qiniu %sv%d","test",7);
}
//*//base_io.c

static void test_Qiniu_Crc32_Update(){
    unsigned long inCrc32,crc;
    const char *buf="Qiniu unit test!";
    size_t bufLen=16;
    inCrc32=3160732;
    crc=Qiniu_Crc32_Update(inCrc32,buf,bufLen);
}

static void test_Qiniu_Crc32Writer(){
    Qiniu_Crc32* crc32=malloc(sizeof(Qiniu_Crc32));
    unsigned long inCrc32=3160732;
    const char *buf="Qiniu unit test!";
    size_t bufLen=16;
    Qiniu_Writer writer;
    writer=Qiniu_Crc32Writer(crc32,inCrc32);
    CU_ASSERT_EQUAL(writer.self,crc32);
    CU_ASSERT_EQUAL(((Qiniu_Crc32*)(writer.self))->val,inCrc32);
    writer.Write(buf,0,bufLen,writer.self);
    CU_ASSERT_EQUAL(((Qiniu_Crc32*)(writer.self))->val,Qiniu_Crc32_Update(inCrc32,buf,bufLen));
}

static void test_Qiniu_BufReader(){
    Qiniu_ReadBuf readBuf;
    Qiniu_Reader reader;
    const char* buf="Qiniu unit test!";
    int len=16;
    reader=Qiniu_BufReader(&readBuf,buf,len);
    CU_ASSERT_EQUAL(reader.self,&readBuf);
    CU_ASSERT_STRING_EQUAL(((Qiniu_ReadBuf*)(reader.self))->buf,buf);
    CU_ASSERT_EQUAL(((Qiniu_ReadBuf*)(reader.self))->off,0);
    CU_ASSERT_EQUAL(((Qiniu_ReadBuf*)(reader.self))->limit,len);
}

static void test_Qiniu_ReadBuf_Read(){
    int len;
    void *buf=malloc(64);

    Qiniu_ReadBuf readBuf;
    Qiniu_Reader reader;
    reader=Qiniu_BufReader(&readBuf,"Qiniu unit test!",16);
    //test branch:n<max
    int buflen1=10;
	len=reader.Read(buf,0,buflen1,reader.self);
	*((char*)buf+len)='\0';
	CU_ASSERT_EQUAL(len,buflen1);
	CU_ASSERT_EQUAL(readBuf.off,buflen1);
	CU_ASSERT_STRING_EQUAL(buf,"Qiniu unit");
    //test branch:n>max

    int buflen2=16;
	len=reader.Read(buf,0,buflen2*2,reader.self);
	CU_ASSERT_EQUAL(len,buflen2-buflen1);
	CU_ASSERT_EQUAL(readBuf.off,buflen2);
	*((char*)buf+len)='\0';
	CU_ASSERT_STRING_EQUAL(buf," test!");
	//test branch:max<=0
	len=reader.Read(buf,0,0,reader.self);
	CU_ASSERT_EQUAL(len,0);
	len=reader.Read(buf,0,-1,reader.self);
	CU_ASSERT_EQUAL(len,0);
}

static void test_Qiniu_BufReaderAt(){
    Qiniu_ReadBuf readBuf;
    Qiniu_ReaderAt readerAt;
    const char* buf="Qiniu unit test!";
    int len=16;
    readerAt=Qiniu_BufReaderAt(&readBuf,buf,len);
    CU_ASSERT_EQUAL(readerAt.self,&readBuf);
    CU_ASSERT_STRING_EQUAL(((Qiniu_ReadBuf*)(readerAt.self))->buf,buf);
    CU_ASSERT_EQUAL(((Qiniu_ReadBuf*)(readerAt.self))->off,0);
    CU_ASSERT_EQUAL(((Qiniu_ReadBuf*)(readerAt.self))->limit,len);
}

static void test_Qiniu_ReadBuf_ReadAt(){
    void *ret=malloc(64);

    Qiniu_ReadBuf readBuf;
    Qiniu_ReaderAt readerAt;
    const char* buf="Qiniu unit test!";
    int len=16;
    readerAt=Qiniu_BufReaderAt(&readBuf,buf,len);
    //test branch:n<max
    int off;
    int retlen;

    off=0;
    len=10;
	retlen=readerAt.ReadAt(readerAt.self,ret,len,off);
	*((char*)ret+retlen)='\0';
	CU_ASSERT_EQUAL(retlen,len);
	CU_ASSERT_STRING_EQUAL(ret,"Qiniu unit");
    //test branch:n>max

    off=11;
    len=5;
	retlen=readerAt.ReadAt(readerAt.self,ret,len*2,off);
	CU_ASSERT_EQUAL(retlen,len);
	*((char*)ret+retlen)='\0';
	CU_ASSERT_STRING_EQUAL(ret,"test!");
	//test branch:max<=0
	retlen=readerAt.ReadAt(readerAt.self,ret,len,17);//off>limit
	CU_ASSERT_EQUAL(retlen,0);
	retlen=readerAt.ReadAt(readerAt.self,ret,-1,0);//unsigned int so -1=2^32-1
	CU_ASSERT_EQUAL(retlen,16);
	*((char*)ret+retlen)='\0';
	CU_ASSERT_STRING_EQUAL(ret,buf);
}

static void test_Qiniu_SectionReader(){
    Qiniu_Section section;
    Qiniu_Reader reader;

    Qiniu_ReadBuf readBuf;
    Qiniu_ReaderAt readerAt;
    const char* buf="Qiniu unit test!";
    int len=16;
    readerAt=Qiniu_BufReaderAt(&readBuf,buf,len);

    reader=Qiniu_SectionReader(&section,readerAt,11,5);
    Qiniu_Section *readerSection=reader.self;
    CU_ASSERT_EQUAL(readerSection,&section);
    CU_ASSERT_EQUAL(readerSection->off,11);
    CU_ASSERT_EQUAL(readerSection->limit,16);
}

static void test_Qiniu_Section_Read(){

    Qiniu_Section section;
    Qiniu_Reader reader;

    Qiniu_ReadBuf readBuf;
    Qiniu_ReaderAt readerAt;
    const char* buf="Qiniu unit test!";
    int len=16;
    readerAt=Qiniu_BufReaderAt(&readBuf,buf,len);

    reader=Qiniu_SectionReader(&section,readerAt,0,16);
    Qiniu_Section *readerSection=reader.self;

    char *ret=malloc(128);
    int retlen;
    int buflen1=10;
	retlen=reader.Read(ret,0,buflen1,reader.self);
	*((char*)ret+retlen)='\0';
	CU_ASSERT_EQUAL(retlen,buflen1);
	CU_ASSERT_EQUAL(readerSection->off,buflen1);
	CU_ASSERT_STRING_EQUAL(ret,"Qiniu unit");
    //test branch:n>max

    int buflen2=16;
	retlen=reader.Read(ret,0,buflen2*2,reader.self);
	CU_ASSERT_EQUAL(retlen,buflen2-buflen1);
	CU_ASSERT_EQUAL(readerSection->off,buflen2);
	*((char*)ret+retlen)='\0';
	CU_ASSERT_STRING_EQUAL(ret," test!");
	//test branch:max<=0
	retlen=reader.Read(ret,0,0,reader.self);
	CU_ASSERT_EQUAL(retlen,0);
	retlen=reader.Read(ret,0,-1,reader.self);
	CU_ASSERT_EQUAL(retlen,0);
}

size_t testRead(void* buf, size_t unused, size_t n, Qiniu_Tee* self){
    return n;
}

size_t testWrite(void* buf, size_t unused, size_t n, Qiniu_Tee* self){
    CU_ASSERT(1);
    return n;
}

static void test_Qiniu_TeeReader(){
    Qiniu_Tee tee;
    Qiniu_Reader r={NULL,(Qiniu_FnRead)testRead};
    Qiniu_Writer w={NULL,(Qiniu_FnWrite)testWrite};
    Qiniu_Reader reader=Qiniu_TeeReader(&tee,r,w);
    CU_ASSERT_EQUAL(reader.self,&tee);
}

static void test_Qiniu_Tee_Read(){
    size_t len;
    Qiniu_Tee tee;
    Qiniu_Reader r={NULL,(Qiniu_FnRead)testRead};
    Qiniu_Writer w={NULL,(Qiniu_FnWrite)testWrite};
    Qiniu_Reader reader=Qiniu_TeeReader(&tee,r,w);
    len=0;
    CU_ASSERT_EQUAL(reader.Read("",0,len,reader.self),len);
    len=3160732;
    CU_ASSERT_EQUAL(reader.Read("",0,len,reader.self),len);

}

static void test_Qiniu_File_Open(){
    Qiniu_Error error;
    Qiniu_File *file;
    error=Qiniu_File_Open(&file,"bazingo");
    CU_ASSERT_STRING_EQUAL(error.message,"open file failed");
    error=Qiniu_File_Open(&file,TESTFILE_I);
    CU_ASSERT_EQUAL(error.code,200);
    Qiniu_File_Close(file);
}

static void test_Qiniu_File_Close(){
    Qiniu_File *file;
    Qiniu_File_Open(&file,TESTFILE_I);
    Qiniu_File_Close(file);
}

static void test_Qiniu_File_Stat(){
    Qiniu_Error error;
    Qiniu_File *file;
    Qiniu_FileInfo* info=malloc(256);
    Qiniu_File_Open(&file,TESTFILE_I);
    error=Qiniu_File_Stat(file,NULL);
    CU_ASSERT_STRING_EQUAL(error.message,"fstat failed");
    error=Qiniu_File_Stat(file,info);
    CU_ASSERT_EQUAL(error.code,200);
}

static void test_Qiniu_FileReaderAt(){
    Qiniu_File *file;
    Qiniu_ReaderAt readerAt;
    readerAt=Qiniu_FileReaderAt(file);
    CU_ASSERT_EQUAL(readerAt.self,file);
}

static void test_Qiniu_File_ReadAt(){
    Qiniu_File *file;
    Qiniu_ReaderAt readerAt;
    Qiniu_File_Open(&file,TESTFILE_I);
    readerAt=Qiniu_FileReaderAt(file);

    char *buf=malloc(128);
    readerAt.ReadAt(readerAt.self,buf,16,0);
    *(buf+16)='\0';
    CU_ASSERT_STRING_EQUAL(buf,"QINIU_UNIT_TEST!");
    readerAt.ReadAt(readerAt.self,buf,32,16);
    *(buf+32)='\0';
    CU_ASSERT_STRING_EQUAL(buf,"QINIU_UNIT_TEST!QINIU_UNIT_TEST!");
}


/**//*---- test suites ------------------*/
static int suite_init(void)
{

	QINIU_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QINIU_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	Qiniu_Global_Init(-1);


	return 0;
}

static int suite_clean(void)
{
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
QINIU_TEST(test_Qiniu_Format_Register)
QINIU_TEST(test_Qiniu_Buffer_AppendFormatV)
QINIU_TEST(test_Qiniu_Buffer_AppendFormat)
QINIU_TEST(test_Qiniu_Buffer_Format)
QINIU_TEST(test_Qiniu_String_Format)
QINIU_TEST(test_Qiniu_FILE_Reader)
QINIU_TEST(test_Qiniu_FILE_Writer)
QINIU_TEST(test_Qiniu_Copy)
QINIU_TEST(test_Qiniu_Null_Fwrite)
QINIU_TEST(test_Qiniu_Null_Log)
QINIU_TEST(test_Qiniu_Logv)
QINIU_TEST(test_Qiniu_Stderr_Info)
QINIU_TEST(test_Qiniu_Stderr_Warn)
//base.c*/
//*base_io.c
QINIU_TEST(test_Qiniu_Crc32_Update)
QINIU_TEST(test_Qiniu_Crc32Writer)
QINIU_TEST(test_Qiniu_BufReader)
QINIU_TEST(test_Qiniu_ReadBuf_Read)
QINIU_TEST(test_Qiniu_BufReaderAt)
QINIU_TEST(test_Qiniu_ReadBuf_ReadAt)
QINIU_TEST(test_Qiniu_SectionReader)
QINIU_TEST(test_Qiniu_Section_Read)
QINIU_TEST(test_Qiniu_TeeReader)
QINIU_TEST(test_Qiniu_Tee_Read)
QINIU_TEST(test_Qiniu_File_Open)
QINIU_TEST(test_Qiniu_File_Close)
QINIU_TEST(test_Qiniu_File_Stat)
QINIU_TEST(test_Qiniu_FileReaderAt)
QINIU_TEST(test_Qiniu_File_ReadAt)
QINIU_TESTS_END()

QINIU_SUITES_BEGIN()
QINIU_SUITE_EX(testcases_base,suite_init,suite_clean)
QINIU_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsBase(void)
{
        QINIU_TEST_REGISTE(base)
}

