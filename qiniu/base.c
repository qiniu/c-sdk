/*
 ============================================================================
 Name        : base.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "base.h"
#include "../b64/urlsafe_b64.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

/*============================================================================*/
/* type Qiniu_Count */

#if defined(_WIN32)

Qiniu_Count Qiniu_Count_Inc(Qiniu_Count* self)
{
	return InterlockedIncrement(self);
}

Qiniu_Count Qiniu_Count_Dec(Qiniu_Count* self)
{
	return InterlockedDecrement(self);
}

#else

Qiniu_Count Qiniu_Count_Inc(Qiniu_Count* self)
{
	return __sync_add_and_fetch(self, 1);
}

Qiniu_Count Qiniu_Count_Dec(Qiniu_Count* self)
{
    return __sync_sub_and_fetch(self, 1);
}

#endif

/*============================================================================*/
/* func Qiniu_Seconds */

Qiniu_Int64 Qiniu_Seconds()
{
	return (Qiniu_Int64)time(NULL);
}

/*============================================================================*/
/* func Qiniu_QueryEscape */

static int Qiniu_shouldEscape(int c)
{
	if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')) {
		return 0;
	}

	switch (c) {
	case '-': case '_': case '.': case '!': case '~':
	case '*': case '\'': case '(': case ')':
		return 0;
	}

	return 1;
}

static const char Qiniu_hexTable[] = "0123456789ABCDEF"; 

char* Qiniu_QueryEscape(const char* s, Qiniu_Bool* fesc)
{
	int spaceCount = 0;
	int hexCount = 0;
	int i, j, len = strlen(s);
	int c;
	char* t;

	for (i = 0; i < len; i++) {
		c = s[i];
		if (Qiniu_shouldEscape(c)) {
			if (c == ' ') {
				spaceCount++;
			} else {
				hexCount++;
			}
		}
	}

	if (spaceCount == 0 && hexCount == 0) {
		*fesc = Qiniu_False;
		return (char*)s;
	}

	t = (char*)malloc(len + 2*hexCount + 1);
	j = 0;
	for (i = 0; i < len; i++) {
		c = s[i];
		if (Qiniu_shouldEscape(c)) {
			if (c == ' ') {
				t[j] = '+';
				j++;
			} else {
				t[j] = '%';
				t[j+1] = Qiniu_hexTable[c>>4];
				t[j+2] = Qiniu_hexTable[c&15];
				j += 3;
			}
		} else {
			t[j] = s[i];
			j++;
		}
	}
	t[j] = '\0';
	*fesc = Qiniu_True;
	return t;
}

/*============================================================================*/
/* func Qiniu_String_Concat */

char* Qiniu_String_Concat2(const char* s1, const char* s2)
{
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char* p = (char*)malloc(len1 + len2 + 1);
	memcpy(p, s1, len1);
	memcpy(p + len1, s2, len2);
	p[len1 + len2] = '\0';
	return p;
}

char* Qiniu_String_Concat3(const char* s1, const char* s2, const char* s3)
{
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	size_t len3 = strlen(s3);
	char* p = (char*)malloc(len1 + len2 + len3 + 1);
	memcpy(p, s1, len1);
	memcpy(p + len1, s2, len2);
	memcpy(p + len1 + len2, s3, len3);
	p[len1 + len2 + len3] = '\0';
	return p;
}

char* Qiniu_String_Concat(const char* s1, ...)
{
	va_list ap;
	char* p;
	const char* s;
	size_t len, slen, len1 = strlen(s1);

	va_start(ap, s1);
	len = len1;
	for (;;) {
		s = va_arg(ap, const char*);
		if (s == NULL) {
			break;
		}
		len += strlen(s);
	}

	p = (char*)malloc(len + 1);

	va_start(ap, s1);
	memcpy(p, s1, len1);
	len = len1;
	for (;;) {
		s = va_arg(ap, const char*);
		if (s == NULL) {
			break;
		}
		slen = strlen(s);
		memcpy(p + len, s, slen);
		len += slen;
	}
	p[len] = '\0';
	return p;
}

/*============================================================================*/
/* func Qiniu_String_Encode */

char* Qiniu_String_Encode(const char* buf)
{
	const size_t cb = strlen(buf);
	const size_t cbDest = urlsafe_b64_encode(buf, cb, NULL, 0);
	char* dest = (char*)malloc(cbDest + 1);
	const size_t cbReal = urlsafe_b64_encode(buf, cb, dest, cbDest);
	dest[cbReal] = '\0';
	return dest;
}

char* Qiniu_Memory_Encode(const char* buf, const size_t cb)
{
	const size_t cbDest = urlsafe_b64_encode(buf, cb, NULL, 0);
	char* dest = (char*)malloc(cbDest + 1);
	const size_t cbReal = urlsafe_b64_encode(buf, cb, dest, cbDest);
	dest[cbReal] = '\0';
	return dest;
}

char* Qiniu_String_Decode(const char* buf)
{
	const size_t cb = strlen(buf);
	const size_t cbDest = urlsafe_b64_decode(buf, cb, NULL, 0);
	char* dest = (char*)malloc(cbDest + 1);
	const size_t cbReal = urlsafe_b64_decode(buf, cb, dest, cbDest);
	dest[cbReal] = '\0';
	return dest;
}

/*============================================================================*/
/* type Qiniu_Buffer */

static void Qiniu_Buffer_expand(Qiniu_Buffer* self, size_t expandSize)
{
	size_t oldSize = self->curr - self->buf;
	size_t newSize = (self->bufEnd - self->buf) << 1;
	expandSize += oldSize;
	while (newSize < expandSize) {
		newSize <<= 1;
	}
	self->buf = realloc(self->buf, newSize);
	self->curr = self->buf + oldSize;
	self->bufEnd = self->buf + newSize;
}

void Qiniu_Buffer_Init(Qiniu_Buffer* self, size_t initSize)
{
	self->buf = self->curr = (char*)malloc(initSize);
	self->bufEnd = self->buf + initSize;
}

void Qiniu_Buffer_Reset(Qiniu_Buffer* self)
{
	self->curr = self->buf;
}

void Qiniu_Buffer_Cleanup(Qiniu_Buffer* self)
{
	if (self->buf != NULL) {
		free(self->buf);
		self->buf = NULL;
	}
}

size_t Qiniu_Buffer_Len(Qiniu_Buffer* self)
{
	return self->curr - self->buf;
}

const char* Qiniu_Buffer_CStr(Qiniu_Buffer* self)
{
	if (self->curr >= self->bufEnd) {
		Qiniu_Buffer_expand(self, 1);
	}
	*self->curr = '\0';
	return self->buf;
}

size_t Qiniu_Buffer_Write(Qiniu_Buffer* self, const void* buf, size_t n)
{
	if (self->curr + n > self->bufEnd) {
		Qiniu_Buffer_expand(self, n);
	}
	memcpy(self->curr, buf, n);
	self->curr += n;
	return n;
}

size_t Qiniu_Buffer_Fwrite(void *buf, size_t size, size_t nmemb, void *self)
{
	assert(size == 1);
	return Qiniu_Buffer_Write((Qiniu_Buffer*)self, buf, nmemb);
}

/*============================================================================*/
/* func Qiniu_Null_Fwrite */

size_t Qiniu_Null_Fwrite(void *buf, size_t size, size_t nmemb, void *self)
{
	return nmemb;
}

/*============================================================================*/
/* func Qiniu_FILE_Reader */

Qiniu_Reader Qiniu_FILE_Reader(FILE* fp)
{
	Qiniu_Reader reader = { fp, (Qiniu_FnRead)fread };
	return reader;
}

/*============================================================================*/

