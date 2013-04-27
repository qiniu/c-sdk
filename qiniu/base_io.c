/*
 ============================================================================
 Name        : base_io.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "base.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef O_BINARY
#define O_BINARY	0
#endif

/*============================================================================*/
/* Qiniu_Buffer_Reader */

static size_t Qiniu_BufReader_Read(void *buf, size_t unused, size_t n, void *self1)
{
	Qiniu_BufReader* self = (Qiniu_BufReader*)self1;
	size_t max = self->limit - self->off;
	if (max <= 0) {
		return 0;
	}
	if (n > max) {
		n = (size_t)max;
	}
	memcpy(buf, self->buf + self->off, n);
	self->off += n;
	return n;
}

Qiniu_Reader Qiniu_Buffer_Reader(Qiniu_BufReader* self, const char* buf, size_t bytes)
{
	Qiniu_Reader ret = {self, Qiniu_BufReader_Read};
	self->buf = buf;
	self->off = 0;
	self->limit = bytes;
	return ret;
}

/*============================================================================*/
/* type Qiniu_File */

Qiniu_Error Qiniu_File_Open(Qiniu_File** pp, const char* file)
{
	Qiniu_Error err = {};
	int fd = open(file, O_BINARY | O_RDONLY, 0644);
	if (fd != -1) {
		*pp = (Qiniu_File*)(size_t)fd;
	} else {
		err.code = errno;
		err.message = "open file failed";
	}
	return err;
}

Qiniu_Error Qiniu_File_Stat(Qiniu_File* self, Qiniu_FileInfo* fi)
{
	Qiniu_Error err = {};
	if (fstat((int)(size_t)self, fi) != 0) {
		err.code = errno;
		err.message = "fstat failed";
	}
	return err;
}

void Qiniu_File_Close(void* self)
{
	close((int)(size_t)self);
}

ssize_t Qiniu_File_ReadAt(void* self, void *buf, size_t bytes, off_t offset)
{
	return pread((int)(size_t)self, buf, bytes, offset);
}

/*============================================================================*/
/* func Qiniu_SectionReader */

typedef struct _Qiniu_sectionReader {
	Qiniu_ReaderAt r;
	off_t off;
	off_t limit;
} Qiniu_sectionReader;

static size_t Qiniu_sectionReader_Read(void *buf, size_t unused, size_t n, void *self1)
{
	Qiniu_sectionReader* self = (Qiniu_sectionReader*)self1;
	off_t max = self->limit - self->off;
	if (max <= 0) {
		return 0;
	}
	if (n > max) {
		n = (size_t)max;
	}
	n = self->r.ReadAt(self->r.self, buf, n, self->off);
	if (n < 0) {
		n = 0;
	}
	self->off += n;
	return n;
}

Qiniu_Reader Qiniu_SectionReader(Qiniu_ReaderAt r, off_t off, off_t n)
{
	Qiniu_Reader ret;
	Qiniu_sectionReader* self = malloc(sizeof(Qiniu_sectionReader));
	self->r = r;
	self->off = off;
	self->limit = off + n;
	ret.self = self;
	ret.Read = Qiniu_sectionReader_Read;
	return ret;
}

void Qiniu_SectionReader_Release(void* f)
{
	free(f);
}

/*============================================================================*/
/* Qiniu_File_ReaderAt */

Qiniu_ReaderAt Qiniu_FileReaderAt(Qiniu_File* self)
{
	Qiniu_ReaderAt ret = {self, Qiniu_File_ReadAt};
	return ret;
}

/*============================================================================*/

