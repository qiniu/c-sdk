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

QINIU_DLLAPI extern void Qiniu_Global_Init(long flags);
QINIU_DLLAPI extern void Qiniu_Global_Cleanup();

QINIU_DLLAPI extern void Qiniu_MacAuth_Init();
QINIU_DLLAPI extern void Qiniu_MacAuth_Cleanup();

QINIU_DLLAPI extern void Qiniu_Servend_Init(long flags);
QINIU_DLLAPI extern void Qiniu_Servend_Cleanup();

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

QINIU_DLLAPI extern void Qiniu_Mutex_Init(Qiniu_Mutex* self);
QINIU_DLLAPI extern void Qiniu_Mutex_Cleanup(Qiniu_Mutex* self);

QINIU_DLLAPI extern void Qiniu_Mutex_Lock(Qiniu_Mutex* self);
QINIU_DLLAPI extern void Qiniu_Mutex_Unlock(Qiniu_Mutex* self);

/*============================================================================*/
/* type Qiniu_Json */

typedef struct cJSON Qiniu_Json;

QINIU_DLLAPI extern const char* Qiniu_Json_GetString(Qiniu_Json* self, const char* key, const char* defval);
QINIU_DLLAPI extern int Qiniu_Json_GetArraySize(Qiniu_Json *self, const char* key, Qiniu_Int64 defval);
QINIU_DLLAPI extern const char* Qiniu_Json_GetStringAt(Qiniu_Json* self, int n, const char* defval);
QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Json_GetUInt32(Qiniu_Json *self, const char *key, Qiniu_Uint32 defval);
QINIU_DLLAPI extern Qiniu_Int64 Qiniu_Json_GetInt64(Qiniu_Json* self, const char* key, Qiniu_Int64 defval);
QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Json_GetUInt64(Qiniu_Json* self, const char* key, Qiniu_Uint64 defval);
QINIU_DLLAPI extern int Qiniu_Json_GetInt(Qiniu_Json *self, const char *key, int defval);
QINIU_DLLAPI extern int Qiniu_Json_GetBoolean(Qiniu_Json* self, const char* key, int defval);
QINIU_DLLAPI extern Qiniu_Json* Qiniu_Json_GetObjectItem(Qiniu_Json* self, const char* key, Qiniu_Json* defval);
QINIU_DLLAPI extern Qiniu_Json* Qiniu_Json_GetArrayItem(Qiniu_Json* self, int n, Qiniu_Json* defval);
QINIU_DLLAPI extern void Qiniu_Json_Destroy(Qiniu_Json* self);

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

QINIU_DLLAPI extern Qiniu_Auth Qiniu_NoAuth;

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

	// Use the following field to specify the average transfer speed in bytes per second (Bps)
	// that the transfer should be below during lowSpeedTime seconds for this SDK to consider
	// it to be too slow and abort.
	long lowSpeedLimit;

	// Use the following field to specify the time in number seconds that
	// the transfer speed should be below the logSpeedLimit for this SDK to consider it
	// too slow and abort.
	long lowSpeedTime;
} Qiniu_Client;

QINIU_DLLAPI extern void Qiniu_Client_InitEx(Qiniu_Client* self, Qiniu_Auth auth, size_t bufSize);
QINIU_DLLAPI extern void Qiniu_Client_Cleanup(Qiniu_Client* self);
QINIU_DLLAPI extern void Qiniu_Client_BindNic(Qiniu_Client* self, const char* nic);
QINIU_DLLAPI extern void Qiniu_Client_SetLowSpeedLimit(Qiniu_Client* self, long lowSpeedLimit, long lowSpeedTime);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Client_Call(Qiniu_Client* self, Qiniu_Json** ret, const char* url);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Client_CallNoRet(Qiniu_Client* self, const char* url);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Client_CallWithBinary(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	Qiniu_Reader body, Qiniu_Int64 bodyLen, const char* mimeType);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Client_CallWithBuffer(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	const char* body, size_t bodyLen, const char* mimeType);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Client_CallWithBuffer2(
	Qiniu_Client* self, Qiniu_Json** ret, const char* url,
	const char* body, size_t bodyLen, const char* mimeType);

/*============================================================================*/
/* func Qiniu_Client_InitNoAuth/InitMacAuth  */

typedef struct _Qiniu_Mac {
	const char* accessKey;
	const char* secretKey;
} Qiniu_Mac;

Qiniu_Auth Qiniu_MacAuth(Qiniu_Mac* mac);

QINIU_DLLAPI extern char* Qiniu_Mac_Sign(Qiniu_Mac* self, char* data);
QINIU_DLLAPI extern char* Qiniu_Mac_SignToken(Qiniu_Mac* self, char* data);

QINIU_DLLAPI extern void Qiniu_Client_InitNoAuth(Qiniu_Client* self, size_t bufSize);
QINIU_DLLAPI extern void Qiniu_Client_InitMacAuth(Qiniu_Client* self, size_t bufSize, Qiniu_Mac* mac);

/*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* QINIU_HTTP_H */

