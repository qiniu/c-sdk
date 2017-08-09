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


/********************************
 *          time : string
 *     val_china : int
 *   val_oversea : int
 ********************************/
typedef struct _Qiniu_CDN_FluxOrBandwidthDataItem {
    char *time;
    int val_china;
    int val_oversea;
} Qiniu_CDN_FluxDataItem, Qiniu_CDN_BandwidthDataItem;

/************************************
 *  domain : string
 *  item_a : array
 *   count : int (item array size)
 ************************************/
typedef struct _Qiniu_CDN_FluxData {
    char *domain;
    Qiniu_CDN_FluxDataItem *item_a;
    int count;
    BOOL hasValue;
} Qiniu_CDN_FluxData;

/************************************
 *  domain : string
 *  item_a : array
 *   count : int (item array size)
 ************************************/
typedef struct _Qiniu_CDN_BandwidthData {
    char *domain;
    Qiniu_CDN_BandwidthDataItem *item_a;
    int count;
    BOOL hasValue;
} Qiniu_CDN_BandwidthData;

/************************************
 *   code : int
 *  error : string
 * data_a : array
 *    num : int (data array size)
 ************************************/
typedef struct _Qiniu_CDN_FluxRet {
    int code;
    const char *error;
    Qiniu_CDN_FluxData *data_a;
    int num;
} Qiniu_CDN_FluxRet;

typedef struct _Qiniu_CDN_BandwidthRet {
    int code;
    const char *error;
    Qiniu_CDN_BandwidthData *data_a;
    int num;
} Qiniu_CDN_BandwidthRet;


QINIU_DLLAPI extern char *Qiniu_CDN_MakeDownloadUrlWithDeadline(const char *key, const char *url,
                                                                Qiniu_Uint64 deadline);


/*============================================================================*/

typedef struct _Qiniu_CDN_RefreshRet {
    int code;
    const char *error;
    const char *requestId;
    char **invalidUrls;
    int invalidUrlsCount;
    char **invalidDirs;
    int invalidDirsCount;
    int urlQuotaDay;
    int urlSurplusDay;
    int dirQuotaDay;
    int dirSurplusDay;

} Qiniu_CDN_RefreshRet;

/* func Qiniu_CDN_RefreshUrls*/
QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_RefreshUrls(Qiniu_Client *self, Qiniu_CDN_RefreshRet *ret, char *urls[],
                                                      const int urlsCount);

/* func Qiniu_CDN_RefreshDirs*/
QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_RefreshDirs(Qiniu_Client *self, Qiniu_CDN_RefreshRet *ret, char *dirs[],
                                                      const int dirsCount);

/*============================================================================*/
typedef struct _Qiniu_CDN_PrefetchRet {
    int code;
    const char *error;
    const char *requestId;
    char **invalidUrls;
    int invalidUrlsCount;
    int quotaDay;
    int surplusDay;

} Qiniu_CDN_PrefetchRet;

/* func Qiniu_CDN_PrefetchUrls*/
QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_PrefetchUrls(Qiniu_Client *self, Qiniu_CDN_PrefetchRet *ret,
                                                       char *urls[], const int num);

/*============================================================================*/

QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_GetFluxData(Qiniu_Client *self, Qiniu_CDN_FluxRet *ret,
                                                      const char *startDate, const char *endDate,
                                                      const char *granularity, char *domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_GetBandwidthData(Qiniu_Client *self, Qiniu_CDN_BandwidthRet *ret,
                                                           const char *startDate, const char *endDate,
                                                           const char *granularity, char *domains[],
                                                           const int num);

/*============================================================================*/

typedef struct _Qiniu_CDN_LogListDataItem {
    const char *name;
    const char *url;
    Qiniu_Int64 size;
    Qiniu_Int64 mtime;
} Qiniu_CDN_LogListDataItem;

typedef struct _Qiniu_CDN_LogListData {
    const char *domain;
    int itemsCount;
    struct _Qiniu_CDN_LogListDataItem *items;
} Qiniu_CDN_LogListData;

typedef struct _Qiniu_CDN_LogListRet {
    int code;
    const char *error;
    struct _Qiniu_CDN_LogListData *data;
    int domainsCount;
} Qiniu_CDN_LogListRet;

/* func Qiniu_CDN_GetLogList*/

QINIU_DLLAPI extern Qiniu_Error Qiniu_CDN_GetLogList(Qiniu_Client *self, Qiniu_CDN_LogListRet *ret,
                                                     char *domains[],const char *day, const int num);

/* func Qiniu_Parse_* */
QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CDNRefreshRet(Qiniu_Json *root, Qiniu_CDN_RefreshRet *ret);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CDNPrefetchRet(Qiniu_Json *root, Qiniu_CDN_PrefetchRet *ret);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CDNFluxRet(Qiniu_Json *root, Qiniu_CDN_FluxRet *ret, char *domains[],
                                                       const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CDNBandwidthRet(Qiniu_Json *root, Qiniu_CDN_BandwidthRet *ret,
                                                            char *domains[], const int num);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Parse_CDNLogListRet(Qiniu_Json *root, Qiniu_CDN_LogListRet *ret,
                                                          char *domains[], const int num);

/* func Qiniu_Free_* */
QINIU_DLLAPI extern void Qiniu_Free_CDNRefreshRet(Qiniu_CDN_RefreshRet *ret);

QINIU_DLLAPI extern void Qiniu_Free_CDNPrefetchRet(Qiniu_CDN_PrefetchRet *ret);

QINIU_DLLAPI extern void Qiniu_Free_CDNFluxRet(Qiniu_CDN_FluxRet *ret);

QINIU_DLLAPI extern void Qiniu_Free_CDNBandwidthRet(Qiniu_CDN_BandwidthRet *ret);

QINIU_DLLAPI extern void Qiniu_Free_CDNLogListRet(Qiniu_CDN_LogListRet *ret);

//=====================================================================

#ifdef __cplusplus
}
#endif

#endif // QINIU_CDN_H
