/*
 ============================================================================
 Name        : base.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_BASE_H
#define QINIU_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================*/
/* func Qiniu_Zero */

#define Qiniu_Zero(v)		memset(&v, 0, sizeof(v))

/*============================================================================*/
/* func Qiniu_snprintf */

#if defined(_MSC_VER)
#define Qiniu_snprintf		_snprintf
#else
#define Qiniu_snprintf		snprintf
#endif

/*============================================================================*/
/* type Qiniu_Int64, Qiniu_Uint32 */

#if defined(_MSC_VER)
typedef _int64 Qiniu_Int64;
#else
typedef long long Qiniu_Int64;
#endif

typedef unsigned int Qiniu_Uint32;
typedef unsigned short Qiniu_Uint16;

/*============================================================================*/
/* type Qiniu_Bool */

typedef int Qiniu_Bool;

enum {
	Qiniu_False = 0,
	Qiniu_True = 1
};

/*============================================================================*/
/* type Qiniu_Error */

typedef struct _Qiniu_Error {
	int code;
	const char* message;
} Qiniu_Error;

/*============================================================================*/
/* type Qiniu_Count */

typedef long Qiniu_Count;

Qiniu_Count Qiniu_Count_Inc(Qiniu_Count* self);
Qiniu_Count Qiniu_Count_Dec(Qiniu_Count* self);

/*============================================================================*/
/* func Qiniu_String_Concat */

char* Qiniu_String_Concat2(const char* s1, const char* s2);
char* Qiniu_String_Concat3(const char* s1, const char* s2, const char* s3);
char* Qiniu_String_Concat(const char* s1, ...);

/*============================================================================*/
/* func Qiniu_String_Encode */

char* Qiniu_Memory_Encode(const char* buf, const size_t cb);
char* Qiniu_String_Encode(const char* s);
char* Qiniu_String_Decode(const char* s);

/*============================================================================*/
/* func Qiniu_QueryEscape */

char* Qiniu_QueryEscape(const char* s, Qiniu_Bool* fesc);

/*============================================================================*/
/* func Qiniu_Seconds */

Qiniu_Int64 Qiniu_Seconds();

/*============================================================================*/
/* type Qiniu_Buffer */

typedef struct _Qiniu_Buffer {
	char* buf;
	char* curr;
	char* bufEnd;
} Qiniu_Buffer;

void Qiniu_Buffer_Init(Qiniu_Buffer* self, size_t initSize);
void Qiniu_Buffer_Reset(Qiniu_Buffer* self);
void Qiniu_Buffer_Cleanup(Qiniu_Buffer* self);

const char* Qiniu_Buffer_CStr(Qiniu_Buffer* self);

size_t Qiniu_Buffer_Len(Qiniu_Buffer* self);
size_t Qiniu_Buffer_Write(Qiniu_Buffer* self, const void* buf, size_t n);
size_t Qiniu_Buffer_Fwrite(void *buf, size_t, size_t n, void *self);

/*============================================================================*/
/* func Qiniu_Null_Fwrite */

size_t Qiniu_Null_Fwrite(void *buf, size_t, size_t n, void *self);

/*============================================================================*/
/* type Qiniu_Reader */

typedef size_t (*Qiniu_FnRead)(void *buf, size_t, size_t n, void *self);

typedef struct _Qiniu_Reader {
	void* self;
	Qiniu_FnRead Read;
} Qiniu_Reader;

Qiniu_Reader Qiniu_FILE_Reader(FILE* fp);

/*============================================================================*/
/* type Qiniu_BufReader */

typedef struct _Qiniu_BufReader {
	const char* buf;
	size_t off;
	size_t limit;
} Qiniu_BufReader;

Qiniu_Reader Qiniu_Buffer_Reader(Qiniu_BufReader* self, const char* buf, size_t bytes);

/*============================================================================*/
/* type Qiniu_ReaderAt */

typedef	ssize_t (*Qiniu_FnReadAt)(void* self, void *buf, size_t bytes, off_t offset);

typedef struct _Qiniu_ReaderAt {
	void* self;
	Qiniu_FnReadAt ReadAt;
} Qiniu_ReaderAt;

Qiniu_Reader Qiniu_SectionReader(Qiniu_ReaderAt readerAt, off_t off, off_t n);
void Qiniu_SectionReader_Release(void* f);

Qiniu_ReaderAt Qiniu_FileReaderAt_Open(const char* file);
void Qiniu_FileReaderAt_Close(void* f);

/*============================================================================*/

#endif /* QINIU_BASE_H */

