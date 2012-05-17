/*
 ============================================================================
 Name        : conf.h
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_CONF_H
#define QBOX_CONF_H

extern const char* QBOX_CLIENT_ID;				// "abcd0c7edcdf914228ed8aa7c6cee2f2bc6155e2"
extern const char* QBOX_CLIENT_SECRET;			// "fc9ef8b171a74e197b17f85ba23799860ddf3b9c"

extern const char* QBOX_REDIRECT_URI;			// "<RedirectURL>"
extern const char* QBOX_AUTHORIZATION_ENDPOINT;	// "<AuthURL>"
extern const char* QBOX_TOKEN_ENDPOINT;			// "https://acc.qbox.me/oauth2/token"

extern int QBOX_PUT_TIMEOUT;					// 300000 = 300s = 5m

extern const char* QBOX_IO_HOST;				// "http://io.qbox.me"
extern const char* QBOX_FS_HOST;				// "https://fs.qbox.me"
extern const char* QBOX_RS_HOST;				// "http://rs.qbox.me:10100"

#endif

