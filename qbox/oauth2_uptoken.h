/*
 ============================================================================
 Name        : oauth2_uptoken.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_OAUTH2_UPTOKEN_H
#define QBOX_OAUTH2_UPTOKEN_H

#ifndef QBOX_OAUTH2_H
#include "oauth2.h"
#endif

/*============================================================================*/
/* type QBox_Token */

void QBox_Client_InitByUpToken(QBox_Client* self, char* uptoken, size_t bufSize);
char* QBox_Client_MakeUpToken(const char* scope, long expires, const char* callbackURL, const char* returnURL);

/*============================================================================*/

#endif /* QBOX_OAUTH2_UPTOKEN_H */

