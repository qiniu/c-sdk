/*
 ============================================================================
 Name        : emu_posix.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */
#ifndef QINIU_EMU_POSIX_H
#define QINIU_EMU_POSIX_H

#include <windows.h>
#include <sys/types.h>

#include "macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*/
/* type ssize_t */

#ifndef _W64
#define _W64
#endif

typedef _W64 int ssize_t;

/*============================================================================*/
/* type Qiniu_File */

#define Qiniu_Posix_Handle			HANDLE
#define Qiniu_Posix_InvalidHandle	INVALID_HANDLE_VALUE

typedef _int64 Emu_Off_T;
typedef struct _Emu_FileInfo {
	Emu_Off_T       st_size;    /* total size, in bytes */
	time_t          st_atime;   /* time of last access */
	time_t          st_mtime;   /* time of last modification */
	time_t          st_ctime;   /* time of last status change */
} Emu_FileInfo;

QINIU_DLLAPI extern Qiniu_Posix_Handle Qiniu_Posix_Open(const char* file, int oflag, int mode);
QINIU_DLLAPI extern ssize_t Qiniu_Posix_Pread2(Qiniu_Posix_Handle fd, void* buf, size_t nbytes, Emu_Off_T offset);
QINIU_DLLAPI extern int Qiniu_Posix_Fstat2(Qiniu_Posix_Handle fd, Emu_FileInfo* buf);
QINIU_DLLAPI extern int Qiniu_Posix_Close(Qiniu_Posix_Handle fd);
QINIU_DLLAPI extern unsigned _int64 Qiniu_Posix_GetTimeOfDay(void);

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* QINIU_EMU_POSIX_H */
