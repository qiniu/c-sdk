/*
 ============================================================================
 Name        : io.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_IO_H
#define QINIU_IO_H

#include "http.h"
#include "reader.h"

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	/*============================================================================*/
	/* type Qiniu_Io_PutExtra */

	typedef struct _Qiniu_Io_PutExtraParam
	{
		const char *key;
		const char *value;
		struct _Qiniu_Io_PutExtraParam *next;
	} Qiniu_Io_PutExtraParam;

	typedef struct _Qiniu_Io_PutExtra
	{
		Qiniu_Io_PutExtraParam *params;
		const char *mimeType;
		Qiniu_Uint32 crc32;
		Qiniu_Uint32 checkCrc32;

		// For those file systems that save file name as Unicode strings,
		// use this field to name the local file name in UTF-8 format for CURL.
		const char *localFileName;

		// For those who want to invoke a upload callback on the business server
		// which returns a JSON object.
		void *callbackRet;
		Qiniu_Error (*callbackRetParser)(void *, Qiniu_Json *);

		// For those who want to abort uploading data to server.
		void *upAbortUserData;
		Qiniu_Rd_FnAbort upAbortCallback;

		// Deprecated fields, prefer upHosts.
		const char *upHost;
		const char **upIps;
		Qiniu_Count ipCount;
		Qiniu_Count ipIndex;

		// Specify multiple upHosts
		const char *const *upHosts;
		size_t upHostsCount;

		// Uploading file progress
		void (*uploadingProgress)(size_t ultotal, size_t ulnow);
	} Qiniu_Io_PutExtra;

	/*============================================================================*/
	/* type Qiniu_Io_PutRet */

	typedef struct _Qiniu_Io_PutRet
	{
		const char *hash;
		const char *key;
		const char *persistentId;
	} Qiniu_Io_PutRet;

	typedef size_t (*rdFunc)(void *buffer, size_t size, size_t n, void *rptr);

	/*============================================================================*/
	/* func Qiniu_Io_PutXXX */

#ifndef QINIU_UNDEFINED_KEY
#define QINIU_UNDEFINED_KEY NULL
#endif

	QINIU_DLLAPI extern Qiniu_Error Qiniu_Io_PutFile(
		Qiniu_Client *self, Qiniu_Io_PutRet *ret,
		const char *uptoken, const char *key, const char *localFile, Qiniu_Io_PutExtra *extra);

	QINIU_DLLAPI extern Qiniu_Error Qiniu_Io_PutBuffer(
		Qiniu_Client *self, Qiniu_Io_PutRet *ret,
		const char *uptoken, const char *key, const char *buf, size_t fsize, Qiniu_Io_PutExtra *extra);

	QINIU_DLLAPI extern Qiniu_Error Qiniu_Io_PutStream(
		Qiniu_Client *self,
		Qiniu_Io_PutRet *ret,
		const char *uptoken, const char *key,
		void *ctx, // 'ctx' is the same as rdr's last param
		size_t fsize,
		rdFunc rdr,
		Qiniu_Io_PutExtra *extra);

	QINIU_DLLAPI extern Qiniu_Auth Qiniu_UptokenAuth(const char *uptoken);
	QINIU_DLLAPI extern void Qiniu_UptokenAuth_Release(void *p);
	QINIU_DLLAPI extern Qiniu_Error Qiniu_UptokenAuth_ToHeader(
		void *self, Qiniu_Header **header, const char *url, const char *addition, size_t addlen);
	/*============================================================================*/

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif // QINIU_IO_H
