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

#define TRUE 1
#define FALSE 0

typedef unsigned char BOOL;

typedef struct _Qiniu_Cdn_RefreshRet {
	int    code;
	char*  error;
	char*  requestId;
	char*  invalidUrls;
	char*  invalidDirs;
	int    urlQuotaDay;
	int    urlSurplusDay;
	int    dirQuotaDay;
	int    dirSurplusDay;

} Qiniu_Cdn_RefreshRet;

typedef struct _Qiniu_Cdn_PrefetchRet {
	int    code;
	char*  error;
	char*  requestId;
	char*  invalidUrls;
	int    quotaDay;
	int    surplusDay;

} Qiniu_Cdn_PrefetchRet;

/********************************
 *          time : string
 *     val_china : int
 *   val_oversea : int
 ********************************/
typedef struct _Qiniu_Cdn_FluxOrBandwidthDataItem {
	char* time;
	int   val_china;
	int   val_oversea;
}Qiniu_Cdn_FluxDataItem, Qiniu_Cdn_BandwidthDataItem;

/************************************
 *  domain : string
 *  item_a : array
 *   count : int (item array size)
 ************************************/
typedef struct _Qiniu_Cdn_FluxData {
	char*                   domain;
	Qiniu_Cdn_FluxDataItem* item_a;
	int                     count;
	BOOL                    hasValue;
} Qiniu_Cdn_FluxData;

/************************************
 *  domain : string
 *  item_a : array
 *   count : int (item array size)
 ************************************/
typedef struct _Qiniu_Cdn_BandwidthData {
	char*                        domain;
	Qiniu_Cdn_BandwidthDataItem* item_a;
	int                          count;
	BOOL                         hasValue;
} Qiniu_Cdn_BandwidthData;

/************************************
 *   code : int
 *  error : string
 * data_a : array
 *    num : int (data array size)
 ************************************/
typedef struct _Qiniu_Cdn_FluxRet {
	int                 code;
	char*               error;
	Qiniu_Cdn_FluxData* data_a;
	int                 num;
}Qiniu_Cdn_FluxRet;

typedef struct _Qiniu_Cdn_BandwidthRet {
	int                      code;
	char*                    error;
	Qiniu_Cdn_BandwidthData* data_a;
	int                      num;
}Qiniu_Cdn_BandwidthRet;

/********************************
 *       name : string
 *       size : int
 *      mtime : int
 *        url : string
 ********************************/
typedef struct _Qiniu_Cdn_LogListDataItem {
	char* name;
	int   size;
	int   mtime;
	char* url;
}Qiniu_Cdn_LogListDataItem;

/************************************
 *   domain : string
 *   item_a : array
 *    count : int (item array size)
 * hasValue : BOOL
 ************************************/
typedef struct _Qiniu_Cdn_LogListData {
	char*                      domain;
	Qiniu_Cdn_LogListDataItem* item_a;
	int                        count;
	BOOL                       hasValue;
}Qiniu_Cdn_LogListData;

/************************************
 *    code : int
 *   error : string
 *  data_a : array
 *     num : int (data array size)
 ************************************/
typedef struct _Qiniu_Cdn_LogListRet {
	int                    code;
	char*                  error;
	Qiniu_Cdn_LogListData* data_a;
	int                    num;
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
// *. Parse/Free
// MODIFIED by fengyh 2017-03-28 11:50
//======================================================================

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_RefreshUrls(Qiniu_Client* self, Qiniu_Cdn_RefreshRet* ret, const char* urls[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_RefreshDirs(Qiniu_Client* self, Qiniu_Cdn_RefreshRet* ret, const char* dirs[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_PrefetchUrls(Qiniu_Client* self, Qiniu_Cdn_PrefetchRet* ret, const char* urls[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetFluxData(Qiniu_Client* self, Qiniu_Cdn_FluxRet* ret,
	const char* startDate, const char* endDate, const char* granularity, const char* domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetBandwidthData(Qiniu_Client* self, Qiniu_Cdn_BandwidthRet* ret,
	const char* startDate, const char* endDate, const char* granularity, const char* domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Cdn_GetLogList(Qiniu_Client* self, Qiniu_Cdn_LogListRet* ret,
	const char* day, const char* domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CdnRefreshRet(Qiniu_Json* root, Qiniu_Cdn_RefreshRet* ret);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CdnPrefetchRet(Qiniu_Json* root, Qiniu_Cdn_PrefetchRet* ret);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CdnFluxRet(Qiniu_Json* root, Qiniu_Cdn_FluxRet* ret, const char* domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CdnBandwidthRet(Qiniu_Json* root, Qiniu_Cdn_BandwidthRet* ret, const char* domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CdnLogListRet(Qiniu_Json* root, Qiniu_Cdn_LogListRet* ret, const char* domains[], const int num);

QINIU_DLLAPI extern void Qiniu_Free_CdnRefreshRet(Qiniu_Cdn_RefreshRet* ret);

QINIU_DLLAPI extern void Qiniu_Free_CdnPrefetchRet(Qiniu_Cdn_PrefetchRet* ret);

QINIU_DLLAPI extern void Qiniu_Free_CdnFluxRet(Qiniu_Cdn_FluxRet* ret);

QINIU_DLLAPI extern void Qiniu_Free_CdnBandwidthRet(Qiniu_Cdn_BandwidthRet* ret);

QINIU_DLLAPI extern void Qiniu_Free_CdnLogListRet(Qiniu_Cdn_LogListRet* ret);

//=====================================================================

#ifdef __cplusplus
}
#endif

#endif // QINIU_CDN_H
