/*
 ============================================================================
 Name        : oauth2.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_OAUTH2_H
#define QBOX_OAUTH2_H

#include "base.h"
#include "conf.h"
#include "../cJSON/cJSON.h"
#include <stdio.h>

/*============================================================================*/
/* Global */

void QBox_Global_Init(long flags);
void QBox_Global_Cleanup();

/*============================================================================*/
/* type QBox_Mutex */

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION QBox_Mutex;
#else
#include <pthread.h>
typedef pthread_mutex_t QBox_Mutex;
#endif

void QBox_Mutex_Init(QBox_Mutex* self);
void QBox_Mutex_Cleanup(QBox_Mutex* self);

void QBox_Mutex_Lock(QBox_Mutex* self);
void QBox_Mutex_Unlock(QBox_Mutex* self);

/*============================================================================*/
/* type QBox_Token */

typedef struct _QBox_Token {
	const char* accessToken;
	const char* refreshToken;
	QBox_Int64 expiry;
	QBox_Mutex mutex;
	QBox_Count nref;
} QBox_Token;

QBox_Token* QBox_Token_New(
	const char* accessToken, const char* refreshToken, QBox_Int64 expiry);

QBox_Count QBox_Token_Acquire(QBox_Token* self);
QBox_Count QBox_Token_Release(QBox_Token* self);

char* QBox_Token_Access(QBox_Token* self, QBox_Int64* expiry);

QBox_Error QBox_Token_Refresh(QBox_Token* self);

QBox_Error QBox_Token_ExchangeByPassword(
	QBox_Token** token, const char* user, const char* passwd);

QBox_Error QBox_Token_ExchangeByRefreshToken(
	QBox_Token** token, const char* refreshToken);

/*============================================================================*/
/* type QBox_Json */

typedef struct cJSON QBox_Json;

const char* QBox_Json_GetString(QBox_Json* self, const char* key, const char* defval);
QBox_Int64 QBox_Json_GetInt64(QBox_Json* self, const char* key, QBox_Int64 defval);

/*============================================================================*/
/* type QBox_Client */


struct QBox_Virtual_Client_FuncTable;

typedef struct _QBox_Client {
	void* curl;
	void* authHeader;
	QBox_Int64 expiry;
	QBox_Json* root;
	QBox_Token* token;
	QBox_Buffer b;

    int refreshToken;

    char const* acsKey;
    char const* scrKey;

    struct QBox_Virtual_Client_FuncTable* vptr;
} QBox_Client;

struct QBox_Virtual_Client_FuncTable
{
    QBox_Error (*QBox_Client_CallWithBinary)(QBox_Client*, QBox_Json**, const char*, FILE*, QBox_Int64);
    QBox_Error (*QBox_Client_Call)(QBox_Client*, QBox_Json**, const char*);
    QBox_Error (*QBox_Client_CallNoRet)(QBox_Client*, const char*);
};

void QBox_Client_Init(QBox_Client* self, QBox_Token* token, size_t bufSize);
void QBox_Client_Init_ByAccessKey(QBox_Client* self, char const* accessKey, char const* secretKey, size_t bufSize);
void QBox_Client_Cleanup(QBox_Client* self);

QBox_Error QBox_Client_Call(QBox_Client* self, QBox_Json** ret, const char* url);
QBox_Error QBox_Client_CallNoRet(QBox_Client* self, const char* url);
QBox_Error QBox_Client_CallWithBinary(
	QBox_Client* self, QBox_Json** ret, const char* url, FILE* body, QBox_Int64 bodyLen);

/*============================================================================*/

#endif /* QBOX_OAUTH2_H */

