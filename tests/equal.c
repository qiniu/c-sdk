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

static char* Qiniu_ReadAll(Qiniu_Reader r, void* buf, size_t n)
{
	Qiniu_Buffer b;
	Qiniu_Writer w;
	Qiniu_Buffer_Init(&b, n);
	w = Qiniu_BufWriter(&b);
	Qiniu_Copy(w, r, buf, n, NULL);
	return (char*)Qiniu_Buffer_CStr(&b);
}

/*============================================================================*/

static int Qiniu_StartWithEx(Qiniu_Reader a, const void* v, size_t n, char* buf)
{
	size_t n1 = a.Read(buf, 1, n, a.self);
	if (n1 != n) {
		return 0;
	}
	if (memcmp(v, buf, n) != 0) {
		return 0;
	}
	return 1;
}

static int Qiniu_StartWith(Qiniu_Reader a, const void* v, size_t n)
{
	char* buf = (char*)malloc(n);
	int result = Qiniu_StartWithEx(a, v, n, buf);
	free(buf);
	return result;
}

/*============================================================================*/

static int Qiniu_IsEqualEx(Qiniu_Reader a, Qiniu_Reader b, void* p1, void* p2, size_t n)
{
	int e1, e2;
	size_t n1, n2;
	for (;;) {
		n1 = a.Read(p1, 1, n, a.self);
		e1 = errno;
		n2 = b.Read(p2, 1, n, b.self);
		e2 = errno;
		if (n1 != n2) {
			return 0;
		}
		if (memcmp(p1, p2, n2) != 0) {
			return 0;
		}
		if (n2 != n) {
			break;
		}
	}
	return (e1 == 0 && e2 == 0);
}

int Qiniu_IsEqual(Qiniu_Reader a, Qiniu_Reader b)
{
	const size_t n = 1024;
	void* p1 = malloc(n);
	void* p2 = malloc(n);
	int result = Qiniu_IsEqualEx(a, b, p1, p2, n);
	free(p2);
	free(p1);
	return result;
}

/*============================================================================*/

size_t Qiniu_Eq_Fwrite(const void* buf, size_t unused, size_t n, Qiniu_Eq* self)
{
	if (self->result) {
		self->result = Qiniu_StartWith(self->v, buf, n);
	}
	return n;
}

int Qiniu_Is(Qiniu_Eq* self)
{
	Qiniu_Int64 ncopy;
	Qiniu_Error err;
	if (self->result) {
		err = Qiniu_Copy(Qiniu_Discard, self->v, NULL, 1024, &ncopy);
		Qiniu_Log_Info("Qiniu_Is: copy.n = %D, copy.err = %E", ncopy, err);
		return err.code == 200 && ncopy == 0;
	}
	return 0;
}

Qiniu_Writer Qiniu_EqWriter(Qiniu_Eq* self, Qiniu_Reader v)
{
	Qiniu_Writer ret = {self, (Qiniu_FnWrite)Qiniu_Eq_Fwrite};
	self->v = v;
	self->result = 1;
	return ret;
}

/*============================================================================*/

