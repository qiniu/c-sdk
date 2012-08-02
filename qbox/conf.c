#include "conf.h"

/*============================================================================*/

const char* QBOX_ACCESS_KEY				= "<Please apply your access key>";
const char* QBOX_SECRET_KEY				= "<Dont send your secret key to anyone>";

const char* QBOX_REDIRECT_URI			= "<RedirectURL>";
const char* QBOX_AUTHORIZATION_ENDPOINT	= "<AuthURL>";
const char* QBOX_TOKEN_ENDPOINT			= "https://acc.qbox.me/oauth2/token";

int QBOX_PUT_TIMEOUT					= 300000; // 300s = 5m
int QBOX_PUT_CHUNK_SIZE					= 256 * 1024; // 256k
int QBOX_PUT_RETRY_TIMES				= 2;



/*
const char* QBOX_IO_HOST				= "http://iovip.qbox.me";
const char* QBOX_FS_HOST				= "https://fs.qbox.me";
const char* QBOX_RS_HOST				= "http://rs.qbox.me:10100";
const char* QBOX_UP_HOST				= "http://up.qbox.me";
const char* QBOX_PU_HOST				= "http://pu.qbox.me:10200";
const char* QBOX_WM_HOST				= "http://m1.qbox.me:15000";
*/

#define QBOX_SDK_DEBUG

#ifndef QBOX_SDK_DEBUG

const char* QBOX_IO_HOST				= "http://iovip.qbox.me";
const char* QBOX_FS_HOST				= "https://fs.qbox.me";
const char* QBOX_RS_HOST				= "http://rs.qbox.me:10100";
const char* QBOX_UP_HOST				= "http://up.qbox.me";
const char* QBOX_PU_HOST				= "http://pu.qbox.me:10200";
const char* QBOX_EU_HOST				= "http://m1.qbox.me:15000";

#else

const char* QBOX_IO_HOST				= "http://m1.qbox.me:13004";
const char* QBOX_UC_HOST				= "http://m1.qbox.me:13010";
const char* QBOX_FS_HOST				= "http://m1.qbox.me:13002";
const char* QBOX_RS_HOST				= "http://m1.qbox.me:13003";
const char* QBOX_UP_HOST				= "http://m1.qbox.me:13019";
const char* QBOX_PU_HOST				= "http://m1.qbox.me:13012";
const char* QBOX_AC_HOST				= "http://m1.qbox.me:13001";
const char* QBOX_EU_HOST				= "http://m1.qbox.me:15000";

#endif

/*============================================================================*/

