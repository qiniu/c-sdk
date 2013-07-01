/*
 ============================================================================
 Name        : base.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "base.h"
#include "../b64/urlsafe_b64.h"
#include <assert.h>
#include <time.h>
#include <errno.h>

/*============================================================================*/
/* type Qiniu_Free */

void Qiniu_Free(void* addr)
{
	free(addr);
}

/*============================================================================*/
/* type Qiniu_Count */

#if defined(_WIN32)

#include <windows.h>

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

typedef enum {
	encodePath,
	encodeUserPassword,
	encodeQueryComponent,
	encodeFragment,
} escapeMode;

// Return true if the specified character should be escaped when
// appearing in a URL string, according to RFC 3986.
static int Qiniu_shouldEscape(int c, escapeMode mode)
{
	if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')) {
		return 0;
	}

	switch (c) {
	case '-': case '_': case '.': case '~': // §2.3 Unreserved characters (mark)
		return 0;
	case '$': case '&': case '+': case ',': case '/': case ':': case ';': case '=': case '?': case '@': // §2.2 Reserved characters (reserved)
		switch (mode) {
			case encodePath: // §3.3
				return c == '?';
			case encodeUserPassword: // §3.2.2
				return c == '@' || c == '/' || c == ':';
			case encodeQueryComponent: // §3.4
				return 1;
			case encodeFragment: // §4.1
				return 0;
		}
	}

	return 1;
}

static const char Qiniu_hexTable[] = "0123456789ABCDEF"; 

static char* Qiniu_escape(const char* s, escapeMode mode, Qiniu_Bool* fesc)
{
	int spaceCount = 0;
	int hexCount = 0;
	int i, j, len = strlen(s);
	int c;
	char* t;

	for (i = 0; i < len; i++) {
		c = s[i];
		if (Qiniu_shouldEscape(c, mode)) {
			if (c == ' ' && mode == encodeQueryComponent) {
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
		if (Qiniu_shouldEscape(c, mode)) {
			if (c == ' ' && mode == encodeQueryComponent) {
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

char* Qiniu_PathEscape(const char* s, Qiniu_Bool* fesc)
{
	return Qiniu_escape(s, encodePath, fesc);
}

char* Qiniu_QueryEscape(const char* s, Qiniu_Bool* fesc)
{
	return Qiniu_escape(s, encodeQueryComponent, fesc);
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

void Qiniu_Buffer_PutChar(Qiniu_Buffer* self, char ch)
{
	if (self->curr >= self->bufEnd) {
		Qiniu_Buffer_expand(self, 1);
	}
	*self->curr++ = ch;
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

size_t Qiniu_Buffer_Fwrite(const void *buf, size_t size, size_t nmemb, void *self)
{
	assert(size == 1);
	return Qiniu_Buffer_Write((Qiniu_Buffer*)self, buf, nmemb);
}

Qiniu_Writer Qiniu_BufWriter(Qiniu_Buffer* self)
{
	Qiniu_Writer writer = {self, Qiniu_Buffer_Fwrite};
	return writer;
}

/*============================================================================*/
/* Qiniu Format Functions */

char* Qiniu_Buffer_Expand(Qiniu_Buffer* self, size_t n)
{
	if (self->curr + n > self->bufEnd) {
		Qiniu_Buffer_expand(self, n);
	}
	return self->curr;
}

void Qiniu_Buffer_Commit(Qiniu_Buffer* self, char* p)
{
	assert(p >= self->curr);
	assert(p <= self->bufEnd);
	self->curr = p;
}

void Qiniu_Buffer_AppendUint(Qiniu_Buffer* self, Qiniu_Uint64 v)
{
	char buf[32];
	char* p = buf+32;
	for (;;) {
		*--p = '0' + (char)(v % 10);
		v /= 10;
		if (v == 0) {
			break;
		}
	}
	Qiniu_Buffer_Write(self, p, buf+32-p);
}

void Qiniu_Buffer_AppendInt(Qiniu_Buffer* self, Qiniu_Int64 v)
{
	if (v < 0) {
		v = -v;
		Qiniu_Buffer_PutChar(self, '-');
	}
	Qiniu_Buffer_AppendUint(self, v);
}

void Qiniu_Buffer_AppendError(Qiniu_Buffer* self, Qiniu_Error v)
{
	Qiniu_Buffer_PutChar(self, 'E');
	Qiniu_Buffer_AppendInt(self, v.code);
	if (v.message) {
		Qiniu_Buffer_PutChar(self, ' ');
		Qiniu_Buffer_Write(self, v.message, strlen(v.message));
	}
}

void Qiniu_Buffer_AppendEncodedBinary(Qiniu_Buffer* self, const char* buf, size_t cb)
{
	const size_t cbDest = urlsafe_b64_encode(buf, cb, NULL, 0);
	char* dest = Qiniu_Buffer_Expand(self, cbDest);
	const size_t cbReal = urlsafe_b64_encode(buf, cb, dest, cbDest);
	Qiniu_Buffer_Commit(self, dest + cbReal);
}

void Qiniu_Buffer_appendUint(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	unsigned v = va_arg(ap->items, unsigned);
	Qiniu_Buffer_AppendUint(self, v);
}

void Qiniu_Buffer_appendInt(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	int v = va_arg(ap->items, int);
	Qiniu_Buffer_AppendInt(self, v);
}

void Qiniu_Buffer_appendUint64(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	Qiniu_Uint64 v = va_arg(ap->items, Qiniu_Uint64);
	Qiniu_Buffer_AppendUint(self, v);
}

void Qiniu_Buffer_appendInt64(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	Qiniu_Int64 v = va_arg(ap->items, Qiniu_Int64);
	Qiniu_Buffer_AppendInt(self, v);
}

void Qiniu_Buffer_appendString(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	const char* v = va_arg(ap->items, const char*);
	if (v == NULL) {
		v = "(null)";
	}
	Qiniu_Buffer_Write(self, v, strlen(v));
}

void Qiniu_Buffer_appendEncodedString(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	const char* v = va_arg(ap->items, const char*);
	size_t n = strlen(v);
	Qiniu_Buffer_AppendEncodedBinary(self, v, n);
}

void Qiniu_Buffer_appendError(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	Qiniu_Error v = va_arg(ap->items, Qiniu_Error);
	Qiniu_Buffer_AppendError(self, v);
}

void Qiniu_Buffer_appendPercent(Qiniu_Buffer* self, Qiniu_Valist* ap)
{
	Qiniu_Buffer_PutChar(self, '%');
}

/*============================================================================*/
/* Qiniu Format */

typedef struct _Qiniu_formatProc {
	Qiniu_FnAppender Append;
	char esc;
} Qiniu_formatProc;

static Qiniu_formatProc qiniu_formatProcs[] = {
	{ Qiniu_Buffer_appendInt, 'd' },
	{ Qiniu_Buffer_appendUint, 'u' },
	{ Qiniu_Buffer_appendInt64, 'D' },
	{ Qiniu_Buffer_appendUint64, 'U' },
	{ Qiniu_Buffer_appendString, 's' },
	{ Qiniu_Buffer_appendEncodedString, 'S' },
	{ Qiniu_Buffer_appendError, 'E' },
	{ Qiniu_Buffer_appendPercent, '%' },
};

static Qiniu_FnAppender qiniu_Appenders[128];

void Qiniu_Format_Register(char esc, Qiniu_FnAppender appender)
{
	if ((unsigned)esc < 128) {
		qiniu_Appenders[esc] = appender;
	}
}

void Qiniu_Buffer_formatInit()
{
	Qiniu_formatProc* p;
	Qiniu_formatProc* pEnd = (Qiniu_formatProc*)((char*)qiniu_formatProcs + sizeof(qiniu_formatProcs));
	for (p = qiniu_formatProcs; p < pEnd; p++) {
		qiniu_Appenders[p->esc] = p->Append;
	}
}

void Qiniu_Buffer_AppendFormatV(Qiniu_Buffer* self, const char* fmt, Qiniu_Valist* args)
{
	unsigned char ch;
	const char* p;
	Qiniu_FnAppender appender;

	for (;;) {
		p = strchr(fmt, '%');
		if (p == NULL) {
			break;
		}
		if (p > fmt) {
			Qiniu_Buffer_Write(self, fmt, p - fmt);
		}
		p++;
		ch = *p++;
		fmt = p;
		if (ch < 128) {
			appender = qiniu_Appenders[ch];
			if (appender != NULL) {
				appender(self, args);
				continue;
			}
		}
		Qiniu_Buffer_PutChar(self, '%');
		Qiniu_Buffer_PutChar(self, ch);
	}
	if (*fmt) {
		Qiniu_Buffer_Write(self, fmt, strlen(fmt));
	}
}

void Qiniu_Buffer_AppendFormat(Qiniu_Buffer* self, const char* fmt, ...)
{
	Qiniu_Valist args;
	va_start(args.items, fmt);
	Qiniu_Buffer_AppendFormatV(self, fmt, &args);
}

const char* Qiniu_Buffer_Format(Qiniu_Buffer* self, const char* fmt, ...)
{
	Qiniu_Valist args;
	va_start(args.items, fmt);
	Qiniu_Buffer_Reset(self);
	Qiniu_Buffer_AppendFormatV(self, fmt, &args);
	return Qiniu_Buffer_CStr(self);
}

char* Qiniu_String_Format(size_t initSize, const char* fmt, ...)
{
	Qiniu_Valist args;
	Qiniu_Buffer buf;
	va_start(args.items, fmt);
	Qiniu_Buffer_Init(&buf, initSize);
	Qiniu_Buffer_AppendFormatV(&buf, fmt, &args);
	return (char*)Qiniu_Buffer_CStr(&buf);
}

/*============================================================================*/
/* func Qiniu_FILE_Reader */

Qiniu_Reader Qiniu_FILE_Reader(FILE* fp)
{
	Qiniu_Reader reader = {fp, (Qiniu_FnRead)fread};
	return reader;
}

Qiniu_Writer Qiniu_FILE_Writer(FILE* fp)
{
	Qiniu_Writer writer = {fp, (Qiniu_FnWrite)fwrite};
	return writer;
}

/*============================================================================*/
/* func Qiniu_Copy */

Qiniu_Error Qiniu_OK = {
	200, "OK"
};

Qiniu_Error Qiniu_Copy(Qiniu_Writer w, Qiniu_Reader r, void* buf, size_t n, Qiniu_Int64* ret)
{
	Qiniu_Int64 fsize = 0;
	size_t n1, n2;
	char* p = (char*)buf;
	if (buf == NULL) {
		p = (char*)malloc(n);
	}
	for (;;) {
		n1 = r.Read(p, 1, n, r.self);
		if (n1 > 0) {
			n2 = w.Write(p, 1, n1, w.self);
			fsize += n2;
		} else {
			n2 = 0;
		}
		if (n2 != n) {
			break;
		}
	}
	if (buf == NULL) {
		free(p);
	}
	if (ret) {
		*ret = fsize;
	}
	return Qiniu_OK;
}

/*============================================================================*/
/* func Qiniu_Null_Fwrite */

size_t Qiniu_Null_Fwrite(const void *buf, size_t size, size_t nmemb, void *self)
{
	return nmemb;
}

Qiniu_Writer Qiniu_Discard = {
	NULL, Qiniu_Null_Fwrite
};

/*============================================================================*/
/* func Qiniu_Null_Log */

void Qiniu_Null_Log(const char* fmt, ...)
{
}

/*============================================================================*/
/* func Qiniu_Stderr_Info/Warn */

static const char* qiniu_Levels[] = {
	"[DEBUG]",
	"[INFO]",
	"[WARN]",
	"[ERROR]",
	"[PANIC]",
	"[FATAL]"
};

void Qiniu_Logv(Qiniu_Writer w, int ilvl, const char* fmt, Qiniu_Valist* args)
{
	const char* level = qiniu_Levels[ilvl];
	Qiniu_Buffer log;
	Qiniu_Buffer_Init(&log, 512);
	Qiniu_Buffer_Write(&log, level, strlen(level));
	Qiniu_Buffer_PutChar(&log, ' ');
	Qiniu_Buffer_AppendFormatV(&log, fmt, args);
	Qiniu_Buffer_PutChar(&log, '\n');
	w.Write(log.buf, 1, log.curr-log.buf, w.self);
	Qiniu_Buffer_Cleanup(&log);
}

void Qiniu_Stderr_Info(const char* fmt, ...)
{
	Qiniu_Valist args;
	va_start(args.items, fmt);
	Qiniu_Logv(Qiniu_Stderr, Qiniu_Linfo, fmt, &args);
}

void Qiniu_Stderr_Warn(const char* fmt, ...)
{
	Qiniu_Valist args;
	va_start(args.items, fmt);
	Qiniu_Logv(Qiniu_Stderr, Qiniu_Lwarn, fmt, &args);
}

/*============================================================================*/

