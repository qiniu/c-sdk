/*
 ============================================================================
 Name        : base.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_BASE_H
#define QINIU_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "macro.h"

/*============================================================================*/
/* func type ssize_t */

#ifdef _WIN32

#include <sys/types.h>

#ifndef _W64
#define _W64
#endif

typedef _W64 int ssize_t;

#endif

#if defined(_MSC_VER)

typedef _int64 Qiniu_Off_T;

#else

typedef off_t Qiniu_Off_T;

#endif

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

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
typedef unsigned _int64 Qiniu_Uint64;
#else
typedef long long Qiniu_Int64;
typedef unsigned long long Qiniu_Uint64;
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

/* @gist error */

typedef struct _Qiniu_Error {
	int code;
	const char* message;
} Qiniu_Error;

/* @endgist */

QINIU_DLLAPI extern Qiniu_Error Qiniu_OK;

/*============================================================================*/
/* type Qiniu_Free */

QINIU_DLLAPI extern void Qiniu_Free(void* addr);

/*============================================================================*/
/* type Qiniu_Count */

typedef long Qiniu_Count;

QINIU_DLLAPI extern Qiniu_Count Qiniu_Count_Inc(Qiniu_Count* self);
QINIU_DLLAPI extern Qiniu_Count Qiniu_Count_Dec(Qiniu_Count* self);

/*============================================================================*/
/* func Qiniu_String_Concat */

QINIU_DLLAPI extern char* Qiniu_String_Concat2(const char* s1, const char* s2);
QINIU_DLLAPI extern char* Qiniu_String_Concat3(const char* s1, const char* s2, const char* s3);
QINIU_DLLAPI extern char* Qiniu_String_Concat(const char* s1, ...);

QINIU_DLLAPI extern char* Qiniu_String_Format(size_t initSize, const char* fmt, ...);

QINIU_DLLAPI extern char* Qiniu_String_Join(const char* deli, char* strs[], int strCount);
QINIU_DLLAPI extern char* Qiniu_String_Dup(const char* src);

/*============================================================================*/
/* func Qiniu_String_Encode */

QINIU_DLLAPI extern char* Qiniu_Memory_Encode(const char* buf, const size_t cb);
QINIU_DLLAPI extern char* Qiniu_String_Encode(const char* s);
QINIU_DLLAPI extern char* Qiniu_String_Decode(const char* s);

/*============================================================================*/
/* func Qiniu_QueryEscape */

char* Qiniu_PathEscape(const char* s, Qiniu_Bool* fesc);
char* Qiniu_QueryEscape(const char* s, Qiniu_Bool* fesc);

/*============================================================================*/
/* func Qiniu_Seconds */

Qiniu_Int64 Qiniu_Seconds();

/*============================================================================*/
/* type Qiniu_Reader */

typedef size_t (*Qiniu_FnRead)(void *buf, size_t, size_t n, void *self);

typedef struct _Qiniu_Reader {
	void* self;
	Qiniu_FnRead Read;
} Qiniu_Reader;

QINIU_DLLAPI extern Qiniu_Reader Qiniu_FILE_Reader(FILE* fp);

/*============================================================================*/
/* type Qiniu_Writer */

typedef size_t (*Qiniu_FnWrite)(const void *buf, size_t, size_t n, void *self);

typedef struct _Qiniu_Writer {
	void* self;
	Qiniu_FnWrite Write;
} Qiniu_Writer;

QINIU_DLLAPI extern Qiniu_Writer Qiniu_FILE_Writer(FILE* fp);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Copy(Qiniu_Writer w, Qiniu_Reader r, void* buf, size_t n, Qiniu_Int64* ret);

#define Qiniu_Stderr Qiniu_FILE_Writer(stderr)

/*============================================================================*/
/* type Qiniu_ReaderAt */

typedef	ssize_t (*Qiniu_FnReadAt)(void* self, void *buf, size_t bytes, Qiniu_Off_T offset);

typedef struct _Qiniu_ReaderAt {
	void* self;
	Qiniu_FnReadAt ReadAt;
} Qiniu_ReaderAt;

/*============================================================================*/
/* type Qiniu_Buffer */

typedef struct _Qiniu_Valist {
	va_list items;
} Qiniu_Valist;

typedef struct _Qiniu_Buffer {
	char* buf;
	char* curr;
	char* bufEnd;
} Qiniu_Buffer;

QINIU_DLLAPI extern void Qiniu_Buffer_Init(Qiniu_Buffer* self, size_t initSize);
QINIU_DLLAPI extern void Qiniu_Buffer_Reset(Qiniu_Buffer* self);
QINIU_DLLAPI extern void Qiniu_Buffer_AppendInt(Qiniu_Buffer* self, Qiniu_Int64 v);
QINIU_DLLAPI extern void Qiniu_Buffer_AppendUint(Qiniu_Buffer* self, Qiniu_Uint64 v);
QINIU_DLLAPI extern void Qiniu_Buffer_AppendError(Qiniu_Buffer* self, Qiniu_Error v);
QINIU_DLLAPI extern void Qiniu_Buffer_AppendEncodedBinary(Qiniu_Buffer* self, const char* buf, size_t cb);
QINIU_DLLAPI extern void Qiniu_Buffer_AppendFormat(Qiniu_Buffer* self, const char* fmt, ...);
QINIU_DLLAPI extern void Qiniu_Buffer_AppendFormatV(Qiniu_Buffer* self, const char* fmt, Qiniu_Valist* args);
QINIU_DLLAPI extern void Qiniu_Buffer_Cleanup(Qiniu_Buffer* self);

QINIU_DLLAPI extern const char* Qiniu_Buffer_CStr(Qiniu_Buffer* self);
QINIU_DLLAPI extern const char* Qiniu_Buffer_Format(Qiniu_Buffer* self, const char* fmt, ...);

QINIU_DLLAPI extern void Qiniu_Buffer_PutChar(Qiniu_Buffer* self, char ch);

QINIU_DLLAPI extern size_t Qiniu_Buffer_Len(Qiniu_Buffer* self);
QINIU_DLLAPI extern size_t Qiniu_Buffer_Write(Qiniu_Buffer* self, const void* buf, size_t n);
QINIU_DLLAPI extern size_t Qiniu_Buffer_Fwrite(const void* buf, size_t, size_t n, void* self);

QINIU_DLLAPI extern Qiniu_Writer Qiniu_BufWriter(Qiniu_Buffer* self);

QINIU_DLLAPI extern char* Qiniu_Buffer_Expand(Qiniu_Buffer* self, size_t n);
QINIU_DLLAPI extern void Qiniu_Buffer_Commit(Qiniu_Buffer* self, char* p);

typedef void (*Qiniu_FnAppender)(Qiniu_Buffer* self, Qiniu_Valist* ap);

QINIU_DLLAPI extern void Qiniu_Format_Register(char esc, Qiniu_FnAppender appender);

/*============================================================================*/
/* func Qiniu_Null_Fwrite */

QINIU_DLLAPI extern size_t Qiniu_Null_Fwrite(const void* buf, size_t, size_t n, void* self);

QINIU_DLLAPI extern Qiniu_Writer Qiniu_Discard;

/*============================================================================*/
/* type Qiniu_ReadBuf */

typedef struct _Qiniu_ReadBuf {
	const char* buf;
	Qiniu_Off_T off;
	Qiniu_Off_T limit;
} Qiniu_ReadBuf;

QINIU_DLLAPI extern Qiniu_Reader Qiniu_BufReader(Qiniu_ReadBuf* self, const char* buf, size_t bytes);
QINIU_DLLAPI extern Qiniu_ReaderAt Qiniu_BufReaderAt(Qiniu_ReadBuf* self, const char* buf, size_t bytes);

/*============================================================================*/
/* type Qiniu_Tee */

typedef struct _Qiniu_Tee {
	Qiniu_Reader r;
	Qiniu_Writer w;
} Qiniu_Tee;

QINIU_DLLAPI extern Qiniu_Reader Qiniu_TeeReader(Qiniu_Tee* self, Qiniu_Reader r, Qiniu_Writer w);

/*============================================================================*/
/* type Qiniu_Section */

typedef struct _Qiniu_Section {
	Qiniu_ReaderAt r;
	Qiniu_Off_T off;
	Qiniu_Off_T limit;
} Qiniu_Section;

QINIU_DLLAPI extern Qiniu_Reader Qiniu_SectionReader(Qiniu_Section* self, Qiniu_ReaderAt r, Qiniu_Off_T off, size_t n);

/*============================================================================*/
/* type Qiniu_Crc32 */

QINIU_DLLAPI extern unsigned long Qiniu_Crc32_Update(unsigned long inCrc32, const void *buf, size_t bufLen);

typedef struct _Qiniu_Crc32 {
	unsigned long val;
} Qiniu_Crc32;

QINIU_DLLAPI extern Qiniu_Writer Qiniu_Crc32Writer(Qiniu_Crc32* self, unsigned long inCrc32);

/*============================================================================*/
/* type Qiniu_File */

typedef struct _Qiniu_File Qiniu_File;

#if defined(_MSC_VER)
typedef struct _Qiniu_FileInfo {
	Qiniu_Off_T     st_size;    /* total size, in bytes */
	time_t          st_atime;   /* time of last access */
	time_t          st_mtime;   /* time of last modification */
	time_t          st_ctime;   /* time of last status change */
} Qiniu_FileInfo;
#else

#include <sys/stat.h>

typedef struct stat Qiniu_FileInfo;

#endif

QINIU_DLLAPI extern Qiniu_Error Qiniu_File_Open(Qiniu_File** pp, const char* file);
QINIU_DLLAPI extern Qiniu_Error Qiniu_File_Stat(Qiniu_File* self, Qiniu_FileInfo* fi);

#define Qiniu_FileInfo_Fsize(fi) ((fi).st_size)

QINIU_DLLAPI extern void Qiniu_File_Close(void* self);

QINIU_DLLAPI extern ssize_t Qiniu_File_ReadAt(void* self, void *buf, size_t bytes, Qiniu_Off_T offset);

QINIU_DLLAPI extern Qiniu_ReaderAt Qiniu_FileReaderAt(Qiniu_File* self);

/*============================================================================*/
/* type Qiniu_Log */

#define Qiniu_Ldebug	0
#define Qiniu_Linfo		1
#define Qiniu_Lwarn		2
#define Qiniu_Lerror	3
#define Qiniu_Lpanic	4
#define Qiniu_Lfatal	5

QINIU_DLLAPI extern void Qiniu_Logv(Qiniu_Writer w, int level, const char* fmt, Qiniu_Valist* args);

QINIU_DLLAPI extern void Qiniu_Stderr_Info(const char* fmt, ...);
QINIU_DLLAPI extern void Qiniu_Stderr_Warn(const char* fmt, ...);

QINIU_DLLAPI extern void Qiniu_Null_Log(const char* fmt, ...);

#ifndef Qiniu_Log_Info

#ifdef QINIU_DISABLE_LOG

#define Qiniu_Log_Info	Qiniu_Null_Log
#define Qiniu_Log_Warn	Qiniu_Null_Log

#else

#define Qiniu_Log_Info	Qiniu_Stderr_Info
#define Qiniu_Log_Warn	Qiniu_Stderr_Warn

#endif

#endif

/*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* QINIU_BASE_H */