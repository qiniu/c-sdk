/*
 ============================================================================
 Name        : oauth2_passwd.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_OAUTH2_PASSWD_H
#define QBOX_OAUTH2_PASSWD_H

#ifndef QBOX_OAUTH2_H
#include "oauth2.h"
#endif

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
/* type QBox_PasswordAuth */

typedef struct _QBox_PasswordAuth {
	QBox_Int64 expiry;
	QBox_Header* authHeader;
	QBox_Token* token;
} QBox_PasswordAuth;

QBox_PasswordAuth* QBox_PasswordAuth_New(QBox_Token* token);

void QBox_Client_InitByPassword(QBox_Client* self, QBox_Token* token, size_t bufSize);

/*============================================================================*/

#endif /* QBOX_OAUTH2_PASSWD_H */

