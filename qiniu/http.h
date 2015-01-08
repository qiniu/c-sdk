/*
 ============================================================================
 Name        : http.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_HTTP_H
#define QINIU_HTTP_H

#include "base.h"
#include "conf.h"

/*============================================================================*/
/* Global */

#ifdef __cplusplus
extern "C"
{
#endif

void Qiniu_Global_Init(long flags);
void Qiniu_Global_Cleanup();

void Qiniu_MacAuth_Init();
void Qiniu_MacAuth_Cleanup();

void Qiniu_Servend_Init(long flags);
void Qiniu_Servend_Cleanup();

#ifdef __cplusplus
}
#endif

/*============================================================================*/
/* type Qiniu_Mutex */

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION Qiniu_Mutex;
#else
#include <pthread.h>
typedef pthread_mutex_t Qiniu_Mutex;
#endif

#ifdef __cplusplus
extern "C"
{
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
/* type Qiniu_Auth */

#pragma pack(1)

typedef struct curl_slist Qiniu_Header;

typedef struct _Qiniu_Auth_Itbl {
	Qiniu_Error (*Auth)(void* self, Qiniu_Header** header, const char* url, const char* addition, size_t addlen);
	void (*Release)(void* self);
} Qiniu_Auth_Itbl;

typedef struct _Qiniu_Auth {
	void* self;
	Qiniu_Auth_Itbl* itbl;
} Qiniu_Auth;

extern Qiniu_Auth Qiniu_NoAuth;

/*============================================================================*/
/* type Qiniu_Client */

typedef struct _Qiniu_Client {
	void* curl;
	Qiniu_Auth auth;
	Qiniu_Json* root;
	Qiniu_Buffer b;
	Qiniu_Buffer respHeader;

	// Use the following field to specify which NIC to use for sending packets.
	const char* boundNic;
} Qiniu_Client;

void Qiniu_Client_InitEx(Qiniu_Client* self, Qiniu_Auth auth, size_t bufSize);
void Qiniu_Client_Cleanup(Qiniu_Client* self);
void Qiniu_Client_BindNic(Qiniu_Client* self, const char* nic);

Qiniu_Error Qiniu_Client_Call(Qiniu_Client* self, Qiniu_Json** ret, const char* url);
Qiniu_Error Qiniu_Client_CallNoRet(Qiniu_Client* self, const char* url);
Qiniu_Error Qiniu_Client_CallWithBinary(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	Qiniu_Reader body, Qiniu_Int64 bodyLen, const char* mimeType);
Qiniu_Error Qiniu_Client_CallWithBuffer(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	const char* body, size_t bodyLen, const char* mimeType);

/*============================================================================*/
/* func Qiniu_Client_InitNoAuth/InitMacAuth  */

typedef struct _Qiniu_Mac {
	const char* accessKey;
	const char* secretKey;
} Qiniu_Mac;

Qiniu_Auth Qiniu_MacAuth(Qiniu_Mac* mac);

char* Qiniu_Mac_Sign(Qiniu_Mac* self, char* data);
char* Qiniu_Mac_SignToken(Qiniu_Mac* self, char* data);

void Qiniu_Client_InitNoAuth(Qiniu_Client* self, size_t bufSize);
void Qiniu_Client_InitMacAuth(Qiniu_Client* self, size_t bufSize, Qiniu_Mac* mac);

/*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* QINIU_HTTP_H */

