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

Qiniu_Reader Qiniu_SeqReader(Qiniu_Seq* self, int radix, int base, size_t delta)
{
	Qiniu_Reader ret = {self, (Qiniu_FnRead)Qiniu_Seq_Read};
	self->radix = radix;
	self->base = base;
	self->delta = delta;
	return ret;
}

/*============================================================================*/

