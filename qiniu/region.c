/*
 ============================================================================
 Name        : region.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "private/region.h"
#include "region.h"
#include "tm.h"
#include "../cJSON/cJSON.h"
#include "../hashmap/hashmap.h"

/*============================================================================*/

#if defined(_WIN32)
#include <windows.h>
#endif

static const char *_Qiniu_Duplicate_Host(const char *host, Qiniu_Bool useHttps)
{
    if (host == NULL)
    {
        return NULL;
    }
    else if (strstr(host, "://") != NULL)
    {
        return Qiniu_String_Dup(host);
    }
    else if (useHttps)
    {
        return Qiniu_String_Concat2("https://", host);
    }
    else
    {
        return Qiniu_String_Concat2("http://", host);
    }
}

static Qiniu_Bool _Qiniu_Region_Service_Copy_Hosts(const char **targetHosts, size_t copyOffset, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    size_t i, j;
    for (i = 0; i < hostsCount; i++)
    {
        if (hosts[i] == NULL)
        {
            break;
        }
        targetHosts[copyOffset + i] = _Qiniu_Duplicate_Host(hosts[i], useHttps);
        if (targetHosts[copyOffset + i] == NULL)
        {
            goto handleErr;
        }
    }
    return Qiniu_True;
handleErr:
    for (j = 0; j < i; j++)
    {
        free((void *)targetHosts[copyOffset + j]);
    }
    return Qiniu_False;
}

struct _Qiniu_Region_Service *_Qiniu_Region_Service_New(
    const char *const *acceleratedHosts, size_t acceleratedHostsCount,
    const char *const *preferredHosts, size_t preferredHostsCount,
    const char *const *alternativeHosts, size_t alternativeHostsCount,
    Qiniu_Bool useHttps)
{
    struct _Qiniu_Region_Service *newService = malloc(sizeof(struct _Qiniu_Region_Service));
    if (newService == NULL)
    {
        goto handleErr;
    }
    size_t allHostsCount = acceleratedHostsCount + preferredHostsCount + alternativeHostsCount;
    const char **newHosts = malloc(sizeof(const char *) * allHostsCount);
    if (newHosts == NULL)
    {
        goto handleErr;
    }
    size_t offset = 0;
    if (_Qiniu_Region_Service_Copy_Hosts(newHosts, offset, acceleratedHosts, acceleratedHostsCount, useHttps) == Qiniu_False)
    {
        goto handleErr;
    }
    offset += acceleratedHostsCount;
    if (_Qiniu_Region_Service_Copy_Hosts(newHosts, offset, preferredHosts, preferredHostsCount, useHttps) == Qiniu_False)
    {
        goto handleErr;
    }
    offset += preferredHostsCount;
    if (_Qiniu_Region_Service_Copy_Hosts(newHosts, offset, alternativeHosts, alternativeHostsCount, useHttps) == Qiniu_False)
    {
        goto handleErr;
    }
    offset += alternativeHostsCount;
    newService->hosts = newHosts;
    newService->acceleratedHostsCount = acceleratedHostsCount;
    newService->preferredHostsCount = preferredHostsCount;
    newService->alternativeHostsCount = alternativeHostsCount;
    return newService;
handleErr:
    if (newHosts != NULL)
    {
        for (size_t j = 0; j < offset; j++)
        {
            free((void *)newHosts[offset + j]);
        }
        free((void *)newHosts);
    }
    if (newService != NULL)
    {
        free((void *)newService);
    }
    return NULL;
}

void _Qiniu_Region_Service_Free(struct _Qiniu_Region_Service *service)
{
    if (service == NULL)
    {
        return;
    }
    size_t allHostsCount = service->acceleratedHostsCount + service->preferredHostsCount + service->alternativeHostsCount;
    for (size_t i = 0; i < allHostsCount; i++)
    {
        free((void *)service->hosts[i]);
    }
    free((void *)service->hosts);
    free((void *)service);
}

Qiniu_Region *Qiniu_Region_New(const char *regionId, Qiniu_Uint64 ttl)
{
    Qiniu_Region *region = malloc(sizeof(Qiniu_Region));
    if (region == NULL)
    {
        return NULL;
    }
    Qiniu_Zero_Ptr(region);
    region->regionId = (const char *)Qiniu_String_Dup(regionId);
    region->expirationTime = Qiniu_Tm_LocalTime() + ttl;
    return region;
}

const char *Qiniu_Region_Get_Id(Qiniu_Region *region)
{
    return region->regionId;
}

Qiniu_Bool Qiniu_Region_Is_Expired(Qiniu_Region *region)
{
    if (region->expirationTime > 0)
    {
        return Qiniu_Tm_LocalTime() > region->expirationTime;
    }
    else
    {
        return Qiniu_False;
    }
}

void Qiniu_Region_Free(Qiniu_Region *region)
{
    Qiniu_FreeV2((void **)&region->regionId);
    region->expirationTime = 0;
    _Qiniu_Region_Service_Free(region->upService);
    region->upService = NULL;
    _Qiniu_Region_Service_Free(region->ioService);
    region->ioService = NULL;
    _Qiniu_Region_Service_Free(region->ioSrcService);
    region->ioSrcService = NULL;
    _Qiniu_Region_Service_Free(region->ucService);
    region->ucService = NULL;
    _Qiniu_Region_Service_Free(region->rsService);
    region->rsService = NULL;
    _Qiniu_Region_Service_Free(region->rsfService);
    region->rsfService = NULL;
    _Qiniu_Region_Service_Free(region->apiService);
    region->apiService = NULL;
    free((void *)region);
}

void Qiniu_Region_Set_Up_Accelerated_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->upService != NULL)
    {
        struct _Qiniu_Region_Service *oldUpService = region->upService;
        region->upService = _Qiniu_Region_Service_New(
            hosts, hostsCount,
            oldUpService->hosts + oldUpService->acceleratedHostsCount, oldUpService->preferredHostsCount,
            oldUpService->hosts + oldUpService->acceleratedHostsCount + oldUpService->preferredHostsCount, oldUpService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldUpService);
    }
    else
    {
        region->upService = _Qiniu_Region_Service_New(hosts, hostsCount, NULL, 0, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Up_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->upService != NULL)
    {
        struct _Qiniu_Region_Service *oldUpService = region->upService;
        region->upService = _Qiniu_Region_Service_New(
            oldUpService->hosts, oldUpService->acceleratedHostsCount,
            hosts, hostsCount,
            oldUpService->hosts + oldUpService->acceleratedHostsCount + oldUpService->preferredHostsCount, oldUpService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldUpService);
    }
    else
    {
        region->upService = _Qiniu_Region_Service_New(NULL, 0, hosts, hostsCount, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Up_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->upService != NULL)
    {
        struct _Qiniu_Region_Service *oldUpService = region->upService;
        region->upService = _Qiniu_Region_Service_New(
            oldUpService->hosts, oldUpService->acceleratedHostsCount,
            oldUpService->hosts + oldUpService->acceleratedHostsCount, oldUpService->preferredHostsCount,
            hosts, hostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldUpService);
    }
    else
    {
        region->upService = _Qiniu_Region_Service_New(NULL, 0, NULL, 0, hosts, hostsCount, useHttps);
    }
}

void Qiniu_Region_Set_Io_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioService != NULL)
    {
        struct _Qiniu_Region_Service *oldIoService = region->ioService;
        region->ioService = _Qiniu_Region_Service_New(
            oldIoService->hosts, oldIoService->acceleratedHostsCount,
            hosts, hostsCount,
            oldIoService->hosts + oldIoService->acceleratedHostsCount + oldIoService->preferredHostsCount, oldIoService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldIoService);
    }
    else
    {
        region->ioService = _Qiniu_Region_Service_New(NULL, 0, hosts, hostsCount, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Io_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioService != NULL)
    {
        struct _Qiniu_Region_Service *oldIoService = region->ioService;
        region->ioService = _Qiniu_Region_Service_New(
            oldIoService->hosts, oldIoService->acceleratedHostsCount,
            oldIoService->hosts + oldIoService->acceleratedHostsCount, oldIoService->preferredHostsCount,
            hosts, hostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldIoService);
    }
    else
    {
        region->ioService = _Qiniu_Region_Service_New(NULL, 0, NULL, 0, hosts, hostsCount, useHttps);
    }
}

void Qiniu_Region_Set_Io_Src_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioSrcService != NULL)
    {
        struct _Qiniu_Region_Service *oldIoSrcService = region->ioSrcService;
        region->ioSrcService = _Qiniu_Region_Service_New(
            oldIoSrcService->hosts, oldIoSrcService->acceleratedHostsCount,
            hosts, hostsCount,
            oldIoSrcService->hosts + oldIoSrcService->acceleratedHostsCount + oldIoSrcService->preferredHostsCount, oldIoSrcService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldIoSrcService);
    }
    else
    {
        region->ioSrcService = _Qiniu_Region_Service_New(NULL, 0, hosts, hostsCount, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Io_Src_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioSrcService != NULL)
    {
        struct _Qiniu_Region_Service *oldIoSrcService = region->ioSrcService;
        region->ioSrcService = _Qiniu_Region_Service_New(
            oldIoSrcService->hosts, oldIoSrcService->acceleratedHostsCount,
            oldIoSrcService->hosts + oldIoSrcService->acceleratedHostsCount, oldIoSrcService->preferredHostsCount,
            hosts, hostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldIoSrcService);
    }
    else
    {
        region->ioSrcService = _Qiniu_Region_Service_New(NULL, 0, NULL, 0, hosts, hostsCount, useHttps);
    }
}

void Qiniu_Region_Set_Bucket_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ucService != NULL)
    {
        struct _Qiniu_Region_Service *oldUcService = region->ucService;
        region->ucService = _Qiniu_Region_Service_New(
            oldUcService->hosts, oldUcService->acceleratedHostsCount,
            hosts, hostsCount,
            oldUcService->hosts + oldUcService->acceleratedHostsCount + oldUcService->preferredHostsCount, oldUcService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldUcService);
    }
    else
    {
        region->ucService = _Qiniu_Region_Service_New(NULL, 0, hosts, hostsCount, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Bucket_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ucService != NULL)
    {
        struct _Qiniu_Region_Service *oldUcService = region->ucService;
        region->ucService = _Qiniu_Region_Service_New(
            oldUcService->hosts, oldUcService->acceleratedHostsCount,
            oldUcService->hosts + oldUcService->acceleratedHostsCount, oldUcService->preferredHostsCount,
            hosts, hostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldUcService);
    }
    else
    {
        region->ucService = _Qiniu_Region_Service_New(NULL, 0, NULL, 0, hosts, hostsCount, useHttps);
    }
}

void Qiniu_Region_Set_Rs_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsService != NULL)
    {
        struct _Qiniu_Region_Service *oldRsService = region->rsService;
        region->rsService = _Qiniu_Region_Service_New(
            oldRsService->hosts, oldRsService->acceleratedHostsCount,
            hosts, hostsCount,
            oldRsService->hosts + oldRsService->acceleratedHostsCount + oldRsService->preferredHostsCount, oldRsService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldRsService);
    }
    else
    {
        region->rsService = _Qiniu_Region_Service_New(NULL, 0, hosts, hostsCount, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Rs_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsService != NULL)
    {
        struct _Qiniu_Region_Service *oldRsService = region->rsService;
        region->rsService = _Qiniu_Region_Service_New(
            oldRsService->hosts, oldRsService->acceleratedHostsCount,
            oldRsService->hosts + oldRsService->acceleratedHostsCount, oldRsService->preferredHostsCount,
            hosts, hostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldRsService);
    }
    else
    {
        region->rsService = _Qiniu_Region_Service_New(NULL, 0, NULL, 0, hosts, hostsCount, useHttps);
    }
}

void Qiniu_Region_Set_Rsf_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsfService != NULL)
    {
        struct _Qiniu_Region_Service *oldRsfService = region->rsfService;
        region->rsfService = _Qiniu_Region_Service_New(
            oldRsfService->hosts, oldRsfService->acceleratedHostsCount,
            hosts, hostsCount,
            oldRsfService->hosts + oldRsfService->acceleratedHostsCount + oldRsfService->preferredHostsCount, oldRsfService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldRsfService);
    }
    else
    {
        region->rsfService = _Qiniu_Region_Service_New(NULL, 0, hosts, hostsCount, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Rsf_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsfService != NULL)
    {
        struct _Qiniu_Region_Service *oldRsfService = region->rsfService;
        region->rsfService = _Qiniu_Region_Service_New(
            oldRsfService->hosts, oldRsfService->acceleratedHostsCount,
            oldRsfService->hosts + oldRsfService->acceleratedHostsCount, oldRsfService->preferredHostsCount,
            hosts, hostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldRsfService);
    }
    else
    {
        region->rsfService = _Qiniu_Region_Service_New(NULL, 0, NULL, 0, hosts, hostsCount, useHttps);
    }
}

void Qiniu_Region_Set_Api_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->apiService != NULL)
    {
        struct _Qiniu_Region_Service *oldApiService = region->apiService;
        region->apiService = _Qiniu_Region_Service_New(
            oldApiService->hosts, oldApiService->acceleratedHostsCount,
            hosts, hostsCount,
            oldApiService->hosts + oldApiService->acceleratedHostsCount + oldApiService->preferredHostsCount, oldApiService->alternativeHostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldApiService);
    }
    else
    {
        region->apiService = _Qiniu_Region_Service_New(NULL, 0, hosts, hostsCount, NULL, 0, useHttps);
    }
}

void Qiniu_Region_Set_Api_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->apiService != NULL)
    {
        struct _Qiniu_Region_Service *oldApiService = region->apiService;
        region->apiService = _Qiniu_Region_Service_New(
            oldApiService->hosts, oldApiService->acceleratedHostsCount,
            oldApiService->hosts + oldApiService->acceleratedHostsCount, oldApiService->preferredHostsCount,
            hosts, hostsCount,
            useHttps);
        _Qiniu_Region_Service_Free(oldApiService);
    }
    else
    {
        region->apiService = _Qiniu_Region_Service_New(NULL, 0, NULL, 0, hosts, hostsCount, useHttps);
    }
}

const char *const *Qiniu_Region_Get_Up_Accelerated_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->upService == NULL)
    {
        return NULL;
    }
    if (region->upService->hosts == NULL || region->upService->acceleratedHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->upService->acceleratedHostsCount;
    }
    return region->upService->hosts;
}

const char *const *Qiniu_Region_Get_Up_Preferred_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->upService == NULL)
    {
        return NULL;
    }
    if (region->upService->hosts == NULL || region->upService->preferredHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->upService->preferredHostsCount;
    }
    return region->upService->hosts + region->upService->acceleratedHostsCount;
}

const char *const *Qiniu_Region_Get_Up_Alternative_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->upService == NULL)
    {
        return NULL;
    }
    if (region->upService->hosts == NULL || region->upService->alternativeHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->upService->alternativeHostsCount;
    }
    return region->upService->hosts + region->upService->acceleratedHostsCount + region->upService->preferredHostsCount;
}

const char *const *_Qiniu_Region_Get_All_Up_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->upService == NULL)
    {
        return NULL;
    }
    size_t allHostsCount = region->upService->preferredHostsCount + region->upService->alternativeHostsCount;
    if (accelerationEnabled)
    {
        allHostsCount += region->upService->acceleratedHostsCount;
    }
    if (region->upService->hosts == NULL || allHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = allHostsCount;
    }
    if (accelerationEnabled)
    {
        return region->upService->hosts;
    }
    else
    {
        return region->upService->hosts + region->upService->acceleratedHostsCount;
    }
}

const char *const *Qiniu_Region_Get_Io_Preferred_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ioService == NULL)
    {
        return NULL;
    }
    if (region->ioService->hosts == NULL || region->ioService->preferredHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioService->preferredHostsCount;
    }
    return region->ioService->hosts + region->ioService->acceleratedHostsCount;
}

const char *const *Qiniu_Region_Get_Io_Alternative_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ioService == NULL)
    {
        return NULL;
    }
    if (region->ioService->hosts == NULL || region->ioService->alternativeHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioService->alternativeHostsCount;
    }
    return region->ioService->hosts + region->ioService->acceleratedHostsCount + region->ioService->preferredHostsCount;
}

const char *const *_Qiniu_Region_Get_All_Io_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ioService == NULL)
    {
        return NULL;
    }
    size_t allHostsCount = region->ioService->preferredHostsCount + region->ioService->alternativeHostsCount;
    if (accelerationEnabled)
    {
        allHostsCount += region->ioService->acceleratedHostsCount;
    }
    if (region->ioService->hosts == NULL || allHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = allHostsCount;
    }
    if (accelerationEnabled)
    {
        return region->ioService->hosts;
    }
    else
    {
        return region->ioService->hosts + region->ioService->acceleratedHostsCount;
    }
}

const char *const *Qiniu_Region_Get_Io_Src_Preferred_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ioSrcService == NULL)
    {
        return NULL;
    }
    if (region->ioSrcService->hosts == NULL || region->ioSrcService->preferredHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioSrcService->preferredHostsCount;
    }
    return region->ioSrcService->hosts + region->ioSrcService->acceleratedHostsCount;
}

const char *const *Qiniu_Region_Get_Io_Src_Alternative_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ioSrcService == NULL)
    {
        return NULL;
    }
    if (region->ioSrcService->hosts == NULL || region->ioSrcService->alternativeHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioSrcService->alternativeHostsCount;
    }
    return region->ioSrcService->hosts + region->ioSrcService->acceleratedHostsCount + region->ioSrcService->preferredHostsCount;
}

const char *const *_Qiniu_Region_Get_All_Io_Src_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ioSrcService == NULL)
    {
        return NULL;
    }
    size_t allHostsCount = region->ioSrcService->preferredHostsCount + region->ioSrcService->alternativeHostsCount;
    if (accelerationEnabled)
    {
        allHostsCount += region->ioSrcService->acceleratedHostsCount;
    }
    if (region->ioSrcService->hosts == NULL || allHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = allHostsCount;
    }
    if (accelerationEnabled)
    {
        return region->ioSrcService->hosts;
    }
    else
    {
        return region->ioSrcService->hosts + region->ioSrcService->acceleratedHostsCount;
    }
}

const char *const *Qiniu_Region_Get_Bucket_Preferred_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ucService == NULL)
    {
        return NULL;
    }
    if (region->ucService->hosts == NULL || region->ucService->preferredHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ucService->preferredHostsCount;
    }
    return region->ucService->hosts + region->ucService->acceleratedHostsCount;
}

const char *const *Qiniu_Region_Get_Bucket_Alternative_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ucService == NULL)
    {
        return NULL;
    }
    if (region->ucService->hosts == NULL || region->ucService->alternativeHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ucService->alternativeHostsCount;
    }
    return region->ucService->hosts + region->ucService->acceleratedHostsCount + region->ucService->preferredHostsCount;
}

const char *const *_Qiniu_Region_Get_All_Bucket_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->ucService == NULL)
    {
        return NULL;
    }
    size_t allHostsCount = region->ucService->preferredHostsCount + region->ucService->alternativeHostsCount;
    if (accelerationEnabled)
    {
        allHostsCount += region->ucService->acceleratedHostsCount;
    }
    if (region->ucService->hosts == NULL || allHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = allHostsCount;
    }
    if (accelerationEnabled)
    {
        return region->ucService->hosts;
    }
    else
    {
        return region->ucService->hosts + region->ucService->acceleratedHostsCount;
    }
}

const char *const *Qiniu_Region_Get_Rs_Preferred_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->rsService == NULL)
    {
        return NULL;
    }
    if (region->rsService->hosts == NULL || region->rsService->preferredHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsService->preferredHostsCount;
    }
    return region->rsService->hosts + region->rsService->acceleratedHostsCount;
}

const char *const *Qiniu_Region_Get_Rs_Alternative_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->rsService == NULL)
    {
        return NULL;
    }
    if (region->rsService->hosts == NULL || region->rsService->alternativeHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsService->alternativeHostsCount;
    }
    return region->rsService->hosts + region->rsService->acceleratedHostsCount + region->rsService->preferredHostsCount;
}

const char *const *_Qiniu_Region_Get_All_Rs_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->rsService == NULL)
    {
        return NULL;
    }
    size_t allHostsCount = region->rsService->preferredHostsCount + region->rsService->alternativeHostsCount;
    if (accelerationEnabled)
    {
        allHostsCount += region->rsService->acceleratedHostsCount;
    }
    if (region->rsService->hosts == NULL || allHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = allHostsCount;
    }
    if (accelerationEnabled)
    {
        return region->rsService->hosts;
    }
    else
    {
        return region->rsService->hosts + region->rsService->acceleratedHostsCount;
    }
}

const char *const *Qiniu_Region_Get_Rsf_Preferred_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->rsfService == NULL)
    {
        return NULL;
    }
    if (region->rsfService->hosts == NULL || region->rsfService->preferredHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsfService->preferredHostsCount;
    }
    return region->rsfService->hosts + region->rsfService->acceleratedHostsCount;
}

const char *const *Qiniu_Region_Get_Rsf_Alternative_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->rsfService == NULL)
    {
        return NULL;
    }
    if (region->rsfService->hosts == NULL || region->rsfService->alternativeHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsfService->alternativeHostsCount;
    }
    return region->rsfService->hosts + region->rsfService->acceleratedHostsCount + region->rsfService->preferredHostsCount;
}

const char *const *_Qiniu_Region_Get_All_Rsf_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->rsfService == NULL)
    {
        return NULL;
    }
    size_t allHostsCount = region->rsfService->preferredHostsCount + region->rsfService->alternativeHostsCount;
    if (accelerationEnabled)
    {
        allHostsCount += region->rsfService->acceleratedHostsCount;
    }
    if (region->rsfService->hosts == NULL || allHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = allHostsCount;
    }
    if (accelerationEnabled)
    {
        return region->rsfService->hosts;
    }
    else
    {
        return region->rsfService->hosts + region->rsfService->acceleratedHostsCount;
    }
}

const char *const *Qiniu_Region_Get_Api_Preferred_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->apiService == NULL)
    {
        return NULL;
    }
    if (region->apiService->hosts == NULL || region->apiService->preferredHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->apiService->preferredHostsCount;
    }
    return region->apiService->hosts + region->apiService->acceleratedHostsCount;
}

const char *const *Qiniu_Region_Get_Api_Alternative_Hosts(Qiniu_Region *region, size_t *count)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->apiService == NULL)
    {
        return NULL;
    }
    if (region->apiService->hosts == NULL || region->apiService->alternativeHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->apiService->alternativeHostsCount;
    }
    return region->apiService->hosts + region->apiService->acceleratedHostsCount + region->apiService->preferredHostsCount;
}

const char *const *_Qiniu_Region_Get_All_Api_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled)
{
    if (count != NULL)
    {
        *count = 0;
    }
    if (region == NULL)
    {
        return NULL;
    }
    if (region->apiService == NULL)
    {
        return NULL;
    }
    size_t allHostsCount = region->apiService->preferredHostsCount + region->apiService->alternativeHostsCount;
    if (accelerationEnabled)
    {
        allHostsCount += region->apiService->acceleratedHostsCount;
    }
    if (region->apiService->hosts == NULL || allHostsCount == 0)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = allHostsCount;
    }
    if (accelerationEnabled)
    {
        return region->apiService->hosts;
    }
    else
    {
        return region->apiService->hosts + region->apiService->acceleratedHostsCount;
    }
}

Qiniu_Region *Qiniu_Use_Region(const char *const regionId, Qiniu_Bool useHttps)
{
    Qiniu_Region *region = Qiniu_Region_New(regionId, 0);
    if (region == NULL)
    {
        return NULL;
    }

    char buf[32], buf2[32];
    Qiniu_Zero(buf);
    Qiniu_Zero(buf2);

    if (strcmp(regionId, "z0") == 0)
    {
        region->upService = _Qiniu_Region_Service_New(NULL, 0, (const char *const[]){"upload.qiniup.com", "up.qiniup.com"}, 2, (const char *const[]){"up.qbox.me"}, 1, useHttps);
        region->ioService = _Qiniu_Region_Service_New(NULL, 0, (const char *const[]){"iovip.qiniuio.com"}, 1, (const char *const[]){"iovip.qbox.me"}, 1, useHttps);
    }
    else
    {
        Qiniu_snprintf(buf, sizeof(buf), "upload-%s.qiniup.com", regionId);
        Qiniu_snprintf(buf2, sizeof(buf2), "up-%s.qiniup.com", regionId);
        region->upService = _Qiniu_Region_Service_New(NULL, 0, (const char *const[]){buf, buf2}, 2, NULL, 0, useHttps);
        Qiniu_snprintf(buf, sizeof(buf), "iovip-%s.qiniuio.com", regionId);
        region->ioService = _Qiniu_Region_Service_New(NULL, 0, (const char *const[]){buf}, 1, NULL, 0, useHttps);
    }
    Qiniu_snprintf(buf, sizeof(buf), "rs-%s.qiniuapi.com", regionId);
    region->rsService = _Qiniu_Region_Service_New(NULL, 0, (const char *const[]){buf}, 1, NULL, 0, useHttps);
    Qiniu_snprintf(buf, sizeof(buf), "rsf-%s.qiniuapi.com", regionId);
    region->rsfService = _Qiniu_Region_Service_New(NULL, 0, (const char *const[]){buf}, 1, NULL, 0, useHttps);
    Qiniu_snprintf(buf, sizeof(buf), "api-%s.qiniuapi.com", regionId);
    region->apiService = _Qiniu_Region_Service_New(NULL, 0, (const char *const[]){buf}, 1, NULL, 0, useHttps);
    return region;
}

static Qiniu_Bool isRegionQueryRetryable(Qiniu_Error err)
{
    switch (err.code)
    {
    case 501:
    case 509:
    case 573:
    case 579:
    case 608:
    case 612:
    case 614:
    case 616:
    case 618:
    case 630:
    case 631:
    case 632:
    case 640:
    case 701:
        return Qiniu_False;
    default:
        return err.code >= 500 || err.code < 100;
    }
}

QINIU_DLLAPI extern const char *QINIU_UC_HOST_BACKUP_2;

static const char **_Qiniu_Get_Bucket_Hosts(Qiniu_Client *self, size_t *count, Qiniu_Bool *needToFree)
{
    const char **hosts = NULL;
    *count = 0;
    *needToFree = Qiniu_False;

    if (self->specifiedRegion == NULL)
    {
        hosts = (const char **)malloc(sizeof(const char *) * 3);
        *needToFree = Qiniu_True;
        if (QINIU_UC_HOST != NULL)
        {
            hosts[(*count)++] = QINIU_UC_HOST;
        }
        if (QINIU_UC_HOST_BACKUP != NULL)
        {
            hosts[(*count)++] = QINIU_UC_HOST_BACKUP;
        }
        if (QINIU_UC_HOST_BACKUP_2 != NULL)
        {
            hosts[(*count)++] = QINIU_UC_HOST_BACKUP_2;
        }
    }
    else
    {
        hosts = (const char **)_Qiniu_Region_Get_All_Bucket_Hosts(self->specifiedRegion, count, Qiniu_False);
    }
    return hosts;
}

static Qiniu_Error _Qiniu_Region_Query_call(Qiniu_Client *self, const char *accessKey, const char *const bucketName, cJSON **ret)
{
    char *url;
    Qiniu_Error err;
    Qiniu_Zero(err);
    const char *const *hosts;
    size_t hostsLen;
    Qiniu_Bool needToFreeHosts;

    hosts = _Qiniu_Get_Bucket_Hosts(self, &hostsLen, &needToFreeHosts);

    for (size_t i = 0; i < hostsLen; i++)
    {
        url = Qiniu_String_Concat(hosts[i], "/v4/query?ak=", accessKey, "&bucket=", bucketName, NULL);
        err = Qiniu_Client_Call(self, ret, url);
        Qiniu_Free(url);
        if (err.code == 200)
        {
            cJSON *hostsJson = Qiniu_Json_GetObjectItem(*ret, "hosts", NULL);
            if (hostsJson == NULL)
            {
                continue;
            }
            cJSON *firstHostJson = Qiniu_Json_GetArrayItem(hostsJson, 0, NULL);
            if (firstHostJson == NULL)
            {
                continue;
            }
            goto handleErr;
        }
        else if (!isRegionQueryRetryable(err))
        {
            goto handleErr;
        }
    }
    if (err.code == 0)
    {
        err.code = 599;
        err.message = "no uc hosts available";
    }
handleErr:
    if (needToFreeHosts)
    {
        Qiniu_Free((void *)hosts);
    }
    return err;
}

static struct _Qiniu_Region_Service *_Qiniu_Region_New_From_JSON(cJSON *json, Qiniu_Bool useHttps)
{
    cJSON *accDomainsJson = Qiniu_Json_GetObjectItem(json, "acc_domains", NULL);
    cJSON *domainsJson = Qiniu_Json_GetObjectItem(json, "domains", NULL);
    cJSON *oldJson = Qiniu_Json_GetObjectItem(json, "old", NULL);
    size_t accDomainsCount = 0, domainsCount = 0, oldCount = 0;
    if (accDomainsJson != NULL)
    {
        accDomainsCount = cJSON_GetArraySize(accDomainsJson);
    }
    if (domainsJson != NULL)
    {
        domainsCount = cJSON_GetArraySize(domainsJson);
    }
    if (oldJson != NULL)
    {
        oldCount = cJSON_GetArraySize(oldJson);
    }

    struct _Qiniu_Region_Service *newService = malloc(sizeof(struct _Qiniu_Region_Service));
    if (newService == NULL)
    {
        goto handleErr;
    }
    const char **newHosts = malloc(sizeof(const char *) * (accDomainsCount + domainsCount + oldCount));
    if (newHosts == NULL)
    {
        goto handleErr;
    }
    size_t offset = 0, i;
    const char *h;
    for (i = 0; i < accDomainsCount; i++)
    {
        h = _Qiniu_Duplicate_Host(Qiniu_Json_GetStringAt(accDomainsJson, i, NULL), useHttps);
        if (h == NULL)
        {
            goto handleErr;
        }
        newHosts[offset + i] = h;
    }
    offset += accDomainsCount;
    for (i = 0; i < domainsCount; i++)
    {
        h = _Qiniu_Duplicate_Host(Qiniu_Json_GetStringAt(domainsJson, i, NULL), useHttps);
        if (h == NULL)
        {
            goto handleErr;
        }
        newHosts[offset + i] = h;
    }
    offset += domainsCount;
    for (i = 0; i < oldCount; i++)
    {
        h = _Qiniu_Duplicate_Host(Qiniu_Json_GetStringAt(oldJson, i, NULL), useHttps);
        if (h == NULL)
        {
            goto handleErr;
        }
        newHosts[offset + i] = h;
    }
    newService->hosts = newHosts;
    newService->acceleratedHostsCount = accDomainsCount;
    newService->preferredHostsCount = domainsCount;
    newService->alternativeHostsCount = oldCount;
    return newService;
handleErr:
    if (newHosts != NULL)
    {
        for (size_t j = 0; j < offset + i; j++)
        {
            free((void *)newHosts[j]);
        }
        free((void *)newHosts);
    }
    if (newService != NULL)
    {
        free((void *)newService);
    }
    return NULL;
}

Qiniu_Error
_Qiniu_Region_Query(Qiniu_Client *self, Qiniu_Region **pRegion, const char *accessKey, const char *const bucketName, Qiniu_Bool useHttps)
{
    Qiniu_Error err;
    cJSON *root;

    if (accessKey == NULL)
    {
        accessKey = self->auth.itbl->GetAccessKey(self->auth.self);
    }
    err = _Qiniu_Region_Query_call(self, accessKey, bucketName, &root);
    if (err.code == 200)
    {
        cJSON *hostsJson = Qiniu_Json_GetObjectItem(root, "hosts", NULL);
        if (hostsJson == NULL)
        {
            err.code = 599;
            err.message = "no hosts in response";
            goto error;
        }

        cJSON *firstHostJson = Qiniu_Json_GetArrayItem(hostsJson, 0, NULL);
        if (firstHostJson == NULL)
        {
            err.code = 599;
            err.message = "empty hosts in response";
            goto error;
        }

        const char *regionId = Qiniu_Json_GetString(firstHostJson, "region", NULL);
        Qiniu_Uint64 ttl = Qiniu_Json_GetInt64(firstHostJson, "ttl", 0);
        Qiniu_Region *region = Qiniu_Region_New(regionId, ttl);

        cJSON *upJson = Qiniu_Json_GetObjectItem(firstHostJson, "up", NULL);
        if (upJson != NULL)
        {
            region->upService = _Qiniu_Region_New_From_JSON(upJson, useHttps);
        }

        cJSON *ioJson = Qiniu_Json_GetObjectItem(firstHostJson, "io", NULL);
        if (ioJson != NULL)
        {
            region->ioService = _Qiniu_Region_New_From_JSON(ioJson, useHttps);
        }

        cJSON *ioSrcJson = Qiniu_Json_GetObjectItem(firstHostJson, "io_src", NULL);
        if (ioSrcJson != NULL)
        {
            region->ioSrcService = _Qiniu_Region_New_From_JSON(ioSrcJson, useHttps);
        }

        cJSON *ucJson = Qiniu_Json_GetObjectItem(firstHostJson, "uc", NULL);
        if (ucJson != NULL)
        {
            region->ucService = _Qiniu_Region_New_From_JSON(ucJson, useHttps);
        }

        cJSON *rsJson = Qiniu_Json_GetObjectItem(firstHostJson, "rs", NULL);
        if (rsJson != NULL)
        {
            region->rsService = _Qiniu_Region_New_From_JSON(rsJson, useHttps);
        }

        cJSON *rsfJson = Qiniu_Json_GetObjectItem(firstHostJson, "rsf", NULL);
        if (rsfJson != NULL)
        {
            region->rsfService = _Qiniu_Region_New_From_JSON(rsfJson, useHttps);
        }

        cJSON *apiJson = Qiniu_Json_GetObjectItem(firstHostJson, "api", NULL);
        if (apiJson != NULL)
        {
            region->apiService = _Qiniu_Region_New_From_JSON(apiJson, useHttps);
        }

        *pRegion = region;
    }

error:
    return err;
}

Qiniu_Error Qiniu_Region_Query(Qiniu_Client *self, Qiniu_Region **pRegion, const char *const bucketName, Qiniu_Bool useHttps)
{
    return _Qiniu_Region_Query(self, pRegion, NULL, bucketName, useHttps);
}

struct _Qiniu_Region_Cache
{
    Qiniu_Region *region;
    const char *accessKey, *bucketName, *const *bucketHosts;
    size_t bucketHostsLen;
    Qiniu_Bool needToFreeBucketHosts, uploadingAccelerationEnabled;
};

static int _Qiniu_Compare_Str(const char *a, const char *b)
{
    if (a == NULL && b == NULL)
    {
        return 0;
    }
    else if (a == NULL)
    {
        return -1;
    }
    else if (b == NULL)
    {
        return 1;
    }
    else
    {
        return strcmp(a, b);
    }
}

static int _Qiniu_Region_Cache_Compare(const void *a, const void *b, void *user_data)
{
    int result;
    struct _Qiniu_Region_Cache *cacheA = (struct _Qiniu_Region_Cache *)a;
    struct _Qiniu_Region_Cache *cacheB = (struct _Qiniu_Region_Cache *)b;
    result = _Qiniu_Compare_Str(cacheA->accessKey, cacheB->accessKey);
    if (!result)
    {
        return result;
    }
    result = _Qiniu_Compare_Str(cacheA->bucketName, cacheB->bucketName);
    if (!result)
    {
        return result;
    }
    if (cacheA->bucketHostsLen == cacheB->bucketHostsLen)
    {
        for (size_t i = 0; i < cacheA->bucketHostsLen; i++)
        {
            result = _Qiniu_Compare_Str(cacheA->bucketHosts[i], cacheB->bucketHosts[i]);
            if (!result)
            {
                return result;
            }
        }
        return result;
    }
    else
    {
        return cacheA->bucketHostsLen - cacheB->bucketHostsLen;
    }
}

static void _Qiniu_Region_Cache_Free(void *r)
{
    struct _Qiniu_Region_Cache *cache = (struct _Qiniu_Region_Cache *)r;
    Qiniu_Free((void *)cache->accessKey);
    Qiniu_Free((void *)cache->bucketName);
    if (cache->needToFreeBucketHosts)
    {
        Qiniu_Free((void *)cache->bucketHosts);
    }
    Qiniu_Region_Free(cache->region);
    Qiniu_Zero_Ptr(cache);
}

static uint64_t _Qiniu_Region_Cache_Hash(const void *r, uint64_t seed0, uint64_t seed1)
{
    struct _Qiniu_Region_Cache *cache = (struct _Qiniu_Region_Cache *)r;
    uint64_t hash = 0;
    if (cache->accessKey != NULL)
    {
        hash ^= hashmap_sip(cache->accessKey, strlen(cache->accessKey), seed0, seed1);
    }
    if (cache->bucketName != NULL)
    {
        hash ^= hashmap_sip(cache->bucketName, strlen(cache->bucketName), seed0, seed1);
    }
    hash ^= hashmap_sip(&cache->bucketHostsLen, sizeof(cache->bucketHostsLen), seed0, seed1);
    for (size_t i = 0; i < cache->bucketHostsLen; i++)
    {
        hash ^= hashmap_sip(cache->bucketHosts[i], strlen(cache->bucketHosts[i]), seed0, seed1);
    }
    hash ^= hashmap_sip(&cache->uploadingAccelerationEnabled, sizeof(cache->uploadingAccelerationEnabled), seed0, seed1);
    return hash;
}

static Qiniu_Error _Qiniu_Region_Auto_Query_With_Cache(Qiniu_Client *self, const char *accessKey, const char *bucketName, Qiniu_Region **foundRegion)
{
    const char *const *hosts;
    size_t hostsLen;
    Qiniu_Bool needToFreeHosts;
    Qiniu_Error err = Qiniu_OK;

    if (accessKey == NULL)
    {
        accessKey = QINIU_ACCESS_KEY;
    }

    hosts = _Qiniu_Get_Bucket_Hosts(self, &hostsLen, &needToFreeHosts);
    const struct _Qiniu_Region_Cache cacheKey = {
        .accessKey = accessKey,
        .bucketName = bucketName,
        .bucketHosts = hosts,
        .bucketHostsLen = hostsLen,
        .needToFreeBucketHosts = needToFreeHosts,
        .uploadingAccelerationEnabled = self->enableUploadingAcceleration,
    };
    struct _Qiniu_Region_Cache *cache = NULL;
    if (self->cachedRegions != NULL)
    {
        cache = (struct _Qiniu_Region_Cache *)hashmap_get(self->cachedRegions, &cacheKey);
        if (cache != NULL && !Qiniu_Region_Is_Expired(cache->region))
        {
            *foundRegion = cache->region;
            err = Qiniu_OK;
            goto handleErr;
        }
    }
    err = _Qiniu_Region_Query(self, foundRegion, accessKey, bucketName, self->autoQueryHttpsRegion);
    if (err.code != 200)
    {
        if (cache != NULL)
        { // 有已经过期的缓存区域可以使用
            *foundRegion = cache->region;
            err = Qiniu_OK;
        }
        goto handleErr;
    }
    if (self->cachedRegions == NULL)
    {
        self->cachedRegions = hashmap_new(sizeof(struct _Qiniu_Region_Cache), 0, rand(), rand(), _Qiniu_Region_Cache_Hash, _Qiniu_Region_Cache_Compare, _Qiniu_Region_Cache_Free, NULL);
    }
    if (self->cachedRegions != NULL)
    {
        if (cache != NULL)
        { // 复用前面已经过期的缓存的内存
            Qiniu_Region_Free(cache->region);
            cache->region = *foundRegion;
        }
        else
        {
            struct _Qiniu_Region_Cache *newCache = malloc(sizeof(struct _Qiniu_Region_Cache));
            if (newCache != NULL)
            {
                *newCache = (struct _Qiniu_Region_Cache){
                    .region = *foundRegion,
                    .accessKey = Qiniu_String_Dup(accessKey),
                    .bucketName = Qiniu_String_Dup(bucketName),
                    .bucketHosts = hosts,
                    .bucketHostsLen = hostsLen,
                    .needToFreeBucketHosts = needToFreeHosts,
                };
                hashmap_set(self->cachedRegions, newCache);
                needToFreeHosts = Qiniu_False;
            }
        }
    }
handleErr:
    if (needToFreeHosts)
    {
        Qiniu_Free((void *)hosts);
    }
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Up_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count)
{
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion && bucketName != NULL)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == 200)
        {
            goto foundCache;
        }
        return err;
    }
foundCache:
    *hosts = _Qiniu_Region_Get_All_Up_Hosts(foundRegion, count, self->enableUploadingAcceleration);
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Io_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count)
{
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion && bucketName != NULL)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == 200)
        {
            goto foundCache;
        }
        return err;
    }
foundCache:
    *hosts = _Qiniu_Region_Get_All_Io_Hosts(foundRegion, count, Qiniu_False);
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Io_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = _Qiniu_Region_Get_Io_Hosts(self, accessKey, bucketName, &hosts, &count);
    if (err.code != 200)
    {
        return err;
    }
    if (count == 0)
    {
        *host = QINIU_IOVIP_HOST;
    }
    else
    {
        *host = hosts[0];
    }
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Rs_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count)
{
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion && bucketName != NULL)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == 200)
        {
            goto foundCache;
        }
        return err;
    }
foundCache:
    *hosts = _Qiniu_Region_Get_All_Rs_Hosts(foundRegion, count, Qiniu_False);
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Rs_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = _Qiniu_Region_Get_Rs_Hosts(self, accessKey, bucketName, &hosts, &count);
    if (err.code != 200)
    {
        return err;
    }
    if (count == 0)
    {
        *host = QINIU_RS_HOST;
    }
    else
    {
        *host = hosts[0];
    }
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Rsf_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count)
{
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion && bucketName != NULL)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == 200)
        {
            goto foundCache;
        }
        return err;
    }
foundCache:
    *hosts = _Qiniu_Region_Get_All_Rsf_Hosts(foundRegion, count, Qiniu_False);
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Rsf_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = _Qiniu_Region_Get_Rsf_Hosts(self, accessKey, bucketName, &hosts, &count);
    if (err.code != 200)
    {
        return err;
    }
    if (count == 0)
    {
        *host = QINIU_RSF_HOST;
    }
    else
    {
        *host = hosts[0];
    }
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Api_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count)
{
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion && bucketName != NULL)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == 200)
        {
            goto foundCache;
        }
        return err;
    }
foundCache:
    *hosts = _Qiniu_Region_Get_All_Api_Hosts(foundRegion, count, Qiniu_False);
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Api_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = _Qiniu_Region_Get_Api_Hosts(self, accessKey, bucketName, &hosts, &count);
    if (err.code != 200)
    {
        return err;
    }
    if (count == 0)
    {
        *host = QINIU_API_HOST;
    }
    else
    {
        *host = hosts[0];
    }
    return err;
}
