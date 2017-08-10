/*
 ============================================================================
 Name        : emu_posix.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "emu_posix.h"
#include <sys/stat.h>

Qiniu_Posix_Handle Qiniu_Posix_Open(const char* file, int oflag, int mode)
{
	Qiniu_Posix_Handle fd = CreateFileA(
		file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (fd != INVALID_HANDLE_VALUE) {
		errno = 0;
		return fd;
	}
	errno = GetLastError();
	return INVALID_HANDLE_VALUE;
}

ssize_t Qiniu_Posix_Pread2(Qiniu_Posix_Handle fd, void* buf, size_t nbytes, Emu_Off_T offset)
{
	BOOL ret;
	DWORD nreaded = 0;
	DWORD tmpErrno = 0;
	OVERLAPPED o = {0};
	o.Offset = (DWORD)(offset & (~(DWORD)0));
	o.OffsetHigh = (DWORD)((offset >> 32) & (~(DWORD)0));
	ret = ReadFile(fd, buf, nbytes, &nreaded, &o);
	if (ret) {
		errno = 0;
		return nreaded;
	}
	tmpErrno = GetLastError();
	if (tmpErrno == ERROR_HANDLE_EOF) {
		// EOF
		return 0;
	}
	errno = tmpErrno;
	return -1;
}

static time_t fileTime2time_t(FILETIME ft)
{
	ULONGLONG ll = ft.dwLowDateTime | ((ULONGLONG)ft.dwHighDateTime << 32);
	return (time_t)((ll - 116444736000000000) / 10000000);
}

int Qiniu_Posix_Fstat2(Qiniu_Posix_Handle fd, Emu_FileInfo* buf)
{
	BY_HANDLE_FILE_INFORMATION fi;
	BOOL ret = GetFileInformationByHandle(fd, &fi);
	if (ret) {
		memset(buf, 0, sizeof(*buf));
		buf->st_size = (Emu_Off_T)fi.nFileSizeLow | ((Emu_Off_T)fi.nFileSizeHigh << 32);
		buf->st_atime = fileTime2time_t(fi.ftLastAccessTime);
		buf->st_mtime = fileTime2time_t(fi.ftLastWriteTime);
		buf->st_ctime = fileTime2time_t(fi.ftCreationTime);
		errno = 0;
		return 0;
	}
	errno = GetLastError();
	return -1;
}

int Qiniu_Posix_Close(Qiniu_Posix_Handle fd)
{
	BOOL ret = CloseHandle(fd);
	if (ret) {
		errno = 0;
		return 0;
	}
	errno = GetLastError();
	return -1;
}

unsigned _int64 Qiniu_Posix_GetTimeOfDay(void)
{
	FILETIME tv;
	ULARGE_INTEGER uint;
	GetSystemTimeAsFileTime(&tv);
	uint.u.LowPart = tv.dwLowDateTime;
	uint.u.HighPart = tv.dwHighDateTime;
	return uint.QuadPart / 1000L;
} // Qiniu_Posix_GetTimeOfDay
