/*
 ============================================================================
 Name        : cdn.h
 Author      : Qiniu.com
 Copyright   : 2017(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_CDN_H
#define QINIU_CDN_H

#include "base.h"
#include "macro.h"
#include "http.h"

 #ifdef __cplusplus
 extern "C"
 {
 #endif

typedef struct _Qiniu_Cdn_RefreshRet {
	int    code;
	char*  error;
	char*  requestId;
	char** invalidUrls;
	char** invalidDirs;
	int    urlQuotaDay;
	int    urlSurplusDay;
	int    dirQuotaDay;
	int    dirSurplusDay;

} Qiniu_Cdn_RefreshRet;

typedef struct _Qiniu_Cdn_PrefetchRet {
	int    code;
	char*  error;
	char*  requestId;
	char** invalidUrls;
	int    quotaDay;
	int    surplusDay;

} Qiniu_Cdn_PrefetchRet;

typedef struct _Qiniu_Cdn_FluxBandwidthRet {
	int    code;
	char*  error;
	char** time;
	void** data;
}Qiniu_Cdn_FluxBandwidthRet;

typedef struct _Qiniu_Cdn_LogListRet {
	int    code;
	char*  error;
	void** data;
}Qiniu_Cdn_LogListRet;

#pragma pack(1)

QINIU_DLLAPI extern char * Qiniu_Cdn_MakeDownloadUrlWithDeadline(const char * key, const char * url, Qiniu_Uint64 deadline);

#pragma pack()

//=====================================================================
// ADDED CDN FUNCTIONS:
// 1. RefreshUrls
// 2. RefreshDirs
// 3. PrefetchUrls
// 4. GetFluxData
// 5. GetBandwidthData
// 6. GetLogList
// MODIFIED by fengyh 2017-03-23 17:26
//======================================================================

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_RefreshUrls(Qiniu_Client* self, Qiniu_Cdn_RefreshRet* ret, const char* urls[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_RefreshDirs(Qiniu_Client* self, Qiniu_Cdn_RefreshRet* ret, const char* dirs[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_PrefetchUrls(Qiniu_Client* self, Qiniu_Cdn_PrefetchRet* ret, const char* urls[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetFluxData(Qiniu_Client* self, Qiniu_Cdn_FluxBandwidthRet* ret,
	const char* startDate, const char* endDate, const char* granularity, const char* domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetBandwidthData(Qiniu_Client* self, Qiniu_Cdn_FluxBandwidthRet* ret,
	const char* startDate, const char* endDate, const char* granularity, const char* domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetLogList(Qiniu_Client* self, Qiniu_Cdn_LogListRet* ret,
	const char* day, const char* domains[], const int num);

//=====================================================================

#ifdef __cplusplus
}
#endif

#endif // QINIU_CDN_H
