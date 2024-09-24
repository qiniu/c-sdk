/*
 ============================================================================
 Name        : region.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_REGION_H
#define QINIU_REGION_H
#include "http.h"

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Up_Preferred_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Up_Alternative_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Up_Accelerated_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Io_Preferred_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Io_Alternative_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Io_Src_Preferred_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Io_Src_Alternative_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Bucket_Preferred_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Bucket_Alternative_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Rs_Preferred_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Rs_Alternative_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Rsf_Preferred_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Rsf_Alternative_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Api_Preferred_Hosts(Qiniu_Region *region, size_t *count);
    QINIU_DLLAPI extern const char *const *Qiniu_Region_Get_Api_Alternative_Hosts(Qiniu_Region *region, size_t *count);

    QINIU_DLLAPI extern Qiniu_Region *Qiniu_Region_New(const char *regionId, Qiniu_Uint64 ttl);
    QINIU_DLLAPI extern const char *Qiniu_Region_Get_Id(Qiniu_Region *region);
    QINIU_DLLAPI extern Qiniu_Bool Qiniu_Region_Is_Expired(Qiniu_Region *region);
    QINIU_DLLAPI extern void Qiniu_Region_Free(Qiniu_Region *region);

    QINIU_DLLAPI extern void Qiniu_Region_Set_Up_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Up_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Up_Accelerated_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Io_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Io_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Io_Src_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Io_Src_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Bucket_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Bucket_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Rs_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Rs_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Rsf_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Rsf_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Api_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern void Qiniu_Region_Set_Api_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);

    QINIU_DLLAPI extern Qiniu_Region *Qiniu_Use_Region(const char *const regionId, Qiniu_Bool useHttps);
    QINIU_DLLAPI extern Qiniu_Error Qiniu_Region_Query(Qiniu_Client *self, Qiniu_Region **region, const char *const bucketName, Qiniu_Bool useHttps);

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif
