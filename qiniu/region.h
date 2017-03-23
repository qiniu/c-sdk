/*
 ============================================================================
 Name		: region.h
 Author	  : Qiniu.com
 Copyright   : 2016(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QINIU_REGION_H
#define QINIU_REGION_H

#include "http.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

QINIU_DLLAPI extern void Qiniu_Rgn_Enable(void);
QINIU_DLLAPI extern void Qiniu_Rgn_Disable(void);
QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_IsEnabled(void);

enum {
	QINIU_RGN_HTTP_HOST         = 0x0001,
	QINIU_RGN_HTTPS_HOST        = 0x0002,
	QINIU_RGN_CDN_HOST          = 0x0004,
	QINIU_RGN_DOWNLOAD_HOST     = 0x0008
};

typedef struct _Qiniu_Rgn_HostInfo {
	const char * host;
	Qiniu_Uint32 flags;
	Qiniu_Uint32 voteCount;
} Qiniu_Rgn_HostInfo;

typedef struct _Qiniu_Rgn_RegionInfo {
	Qiniu_Uint64 nextTimestampToUpdate;

	const char * bucket;

	Qiniu_Int64 ttl;
	Qiniu_Int64 global;

	Qiniu_Uint32 upHostCount;
	Qiniu_Rgn_HostInfo ** upHosts;

	Qiniu_Uint32 ioHostCount;
	Qiniu_Rgn_HostInfo ** ioHosts;
} Qiniu_Rgn_RegionInfo;

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Info_Fetch(Qiniu_Client * cli, Qiniu_Rgn_RegionInfo ** rgnInfo, const char * bucket, const char * accessKey);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Info_FetchByUptoken(Qiniu_Client * cli, Qiniu_Rgn_RegionInfo ** rgnInfo, const char * uptoken);
QINIU_DLLAPI extern void Qiniu_Rgn_Info_Destroy(Qiniu_Rgn_RegionInfo * rgnInfo);
QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_Info_HasExpirated(Qiniu_Rgn_RegionInfo * rgnInfo);
QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_Info_CountUpHost(Qiniu_Rgn_RegionInfo * rgnInfo);
QINIU_DLLAPI extern Qiniu_Uint32 Qiniu_Rgn_Info_CountIoHost(Qiniu_Rgn_RegionInfo * rgnInfo);
QINIU_DLLAPI extern const char * Qiniu_Rgn_Info_GetHost(Qiniu_Rgn_RegionInfo * rgnInfo, Qiniu_Uint32 n, Qiniu_Uint32 hostFlags);
QINIU_DLLAPI extern const char * Qiniu_Rgn_Info_GetIoHost(Qiniu_Rgn_RegionInfo * rgnInfo, Qiniu_Uint32 n, Qiniu_Uint32 hostFlags);

typedef struct _Qiniu_Rgn_RegionTable {
	Qiniu_Uint32 rgnCount;
	Qiniu_Rgn_RegionInfo ** regions;
} Qiniu_Rgn_RegionTable;

typedef struct _Qiniu_Rgn_HostVote {
	Qiniu_Rgn_RegionInfo * rgnInfo;
	Qiniu_Rgn_HostInfo ** host;
	Qiniu_Rgn_HostInfo ** hosts;
	Qiniu_Uint32 hostCount;
	Qiniu_Uint32 hostFlags;
} Qiniu_Rgn_HostVote;

QINIU_DLLAPI extern Qiniu_Rgn_RegionTable * Qiniu_Rgn_Table_Create(void);
QINIU_DLLAPI extern void Qiniu_Rgn_Table_Destroy(Qiniu_Rgn_RegionTable * rgnTable);

QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_FetchAndUpdate(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Client * cli, const char * bucket, const char * access_key);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_FetchAndUpdateByUptoken(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Client * cli, const char * uptoken);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_SetRegionInfo(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Rgn_RegionInfo * rgnInfo);
QINIU_DLLAPI extern Qiniu_Rgn_RegionInfo * Qiniu_Rgn_Table_GetRegionInfo(Qiniu_Rgn_RegionTable * rgnTable, const char * bucket);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_GetHost(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Client * cli, const char * bucket, const char * accessKey, Qiniu_Uint32 hostFlags, const char ** upHost, Qiniu_Rgn_HostVote * vote);
QINIU_DLLAPI extern Qiniu_Error Qiniu_Rgn_Table_GetHostByUptoken(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Client * cli, const char * uptoken, Qiniu_Uint32 hostFlags, const char ** upHost, Qiniu_Rgn_HostVote * vote);
QINIU_DLLAPI extern void Qiniu_Rgn_Table_VoteHost(Qiniu_Rgn_RegionTable * rgnTable, Qiniu_Rgn_HostVote * vote, Qiniu_Error err);

#ifdef __cplusplus
}
#endif

#pragma pack()

#endif // QINIU_REGION_H
