/*
 ============================================================================
 Name        : oauth2.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_OAUTH2_H
#define QINIU_OAUTH2_H

#include "base.h"
#include "conf.h"
#include "../cJSON/cJSON.h"

/*============================================================================*/
/* Global */

void Qiniu_Global_Init(long flags);
void Qiniu_Global_Cleanup();

/*============================================================================*/
/* type Qiniu_Mutex */

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION Qiniu_Mutex;
#else
#include <pthread.h>
typedef pthread_mutex_t Qiniu_Mutex;
#endif

void Qiniu_Mutex_Init(Qiniu_Mutex* self);
void Qiniu_Mutex_Cleanup(Qiniu_Mutex* self);

void Qiniu_Mutex_Lock(Qiniu_Mutex* self);
void Qiniu_Mutex_Unlock(Qiniu_Mutex* self);

/*============================================================================*/
/* type Qiniu_Json */

typedef struct cJSON Qiniu_Json;

const char* Qiniu_Json_GetString(Qiniu_Json* self, const char* key, const char* defval);
Qiniu_Int64 Qiniu_Json_GetInt64(Qiniu_Json* self, const char* key, Qiniu_Int64 defval);

/*============================================================================*/
/* type Qiniu_Client */

typedef struct curl_slist Qiniu_Header;

typedef struct _Qiniu_Auth_Itbl {
	Qiniu_Error (*Auth)(void* self, Qiniu_Header** header, const char* url, const char* addition, size_t addlen);
	void (*Release)(void* self);
} Qiniu_Auth_Itbl;

typedef struct _Qiniu_Auth {
	void* self;
	Qiniu_Auth_Itbl* itbl;
} Qiniu_Auth;

typedef struct _Qiniu_Client {
	void* curl;
	Qiniu_Auth auth;
	Qiniu_Json* root;
	Qiniu_Buffer b;
	Qiniu_Buffer respHeader;
} Qiniu_Client;

void Qiniu_Client_InitEx(Qiniu_Client* self, Qiniu_Auth auth, size_t bufSize);
void Qiniu_Client_Cleanup(Qiniu_Client* self);

Qiniu_Error Qiniu_Client_Call(Qiniu_Client* self, Qiniu_Json** ret, const char* url);
Qiniu_Error Qiniu_Client_CallNoRet(Qiniu_Client* self, const char* url);
Qiniu_Error Qiniu_Client_CallWithBinary(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	Qiniu_Reader body, Qiniu_Int64 bodyLen, const char* mimeType);
Qiniu_Error Qiniu_Client_CallWithBuffer(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	const char* body, size_t bodyLen, const char* mimeType);

/*============================================================================*/

void Qiniu_Client_Init(Qiniu_Client* self, size_t bufSize);
void Qiniu_Client_InitNoAuth(Qiniu_Client* self, size_t bufSize);

/*============================================================================*/

#endif /* QINIU_OAUTH2_H */

