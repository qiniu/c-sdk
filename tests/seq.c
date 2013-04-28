/*
 ============================================================================
 Name        : seq.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "test.h"

/*============================================================================*/
/* type Qiniu_Seq */

static size_t Qiniu_Seq_Read(void *buf, size_t unused, size_t n, Qiniu_Seq* self)
{
	char* p = (char*)buf;
	size_t i, max = self->limit - self->off;
	if (max <= 0) {
		return 0;
	}
	if (n > max) {
		n = (size_t)max;
	}
	for (i = 0; i < n; i++) {
		p[i] = (self->off + self->delta + i) % self->radix + self->base;
	}
	self->off += n;
	return n;
}

static ssize_t Qiniu_Seq_ReadAt(Qiniu_Seq* self, void *buf, size_t n, off_t off)
{
	char* p = (char*)buf;
	size_t i, max = self->limit - (size_t)off;
	if ((ssize_t)max <= 0) {
		return 0;
	}
	if (n > max) {
		n = (size_t)max;
	}
	for (i = 0; i < n; i++) {
		p[i] = (off + self->delta + i) % self->radix + self->base;
	}
	return n;
}

Qiniu_Reader Qiniu_SeqReader(Qiniu_Seq* self, size_t limit, int radix, int base, size_t delta)
{
	Qiniu_Reader ret = {self, (Qiniu_FnRead)Qiniu_Seq_Read};
	self->radix = radix;
	self->base = base;
	self->delta = delta;
	self->off = 0;
	self->limit = limit;
	return ret;
}

Qiniu_ReaderAt Qiniu_SeqReaderAt(Qiniu_Seq* self, size_t limit, int radix, int base, size_t delta)
{
	Qiniu_ReaderAt ret = {self, (Qiniu_FnReadAt)Qiniu_Seq_ReadAt};
	self->radix = radix;
	self->base = base;
	self->delta = delta;
	self->off = 0;
	self->limit = limit;
	return ret;
}

/*============================================================================*/

