/*
 ============================================================================
 Name		: reader.h
 Author	  : Qiniu.com
 Copyright   : 2012-2016(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_READER_H
#define QINIU_READER_H

#include "base.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*/
/* Declaration of Controllable Reader */

typedef int (*Qiniu_Rd_FnAbort)(void * abortUserData, char * buf, size_t size);

enum
{
	QINIU_RD_OK = 0,
	QINIU_RD_ABORT_BY_CALLBACK,
	QINIU_RD_ABORT_BY_READAT
};

typedef struct _Qiniu_Rd_Reader
{
	Qiniu_File * file;
	Qiniu_Off_T offset;

	int status;

	void * abortUserData;
	Qiniu_Rd_FnAbort abortCallback;
} Qiniu_Rd_Reader;

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rd_Reader_Open(Qiniu_Rd_Reader * rdr, const char * localFileName);
QINIU_DLLAPI extern void Qiniu_Rd_Reader_Close(Qiniu_Rd_Reader * rdr);

QINIU_DLLAPI extern size_t Qiniu_Rd_Reader_Callback(char * buffer, size_t size, size_t nitems, void * rdr);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // QINIU_READER_H
