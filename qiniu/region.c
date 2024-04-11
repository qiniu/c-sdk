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

static const char *
duplicate_host(const char *host, Qiniu_Bool useHttps)
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

struct _Qiniu_Region_Hosts *_Qiniu_Region_Hosts_New(const char *const *const hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    const char **newHosts = malloc(sizeof(const char *) * hostsCount);
    if (newHosts == NULL)
    {
        return NULL;
    }

    struct _Qiniu_Region_Hosts *newRegionHosts = malloc(sizeof(struct _Qiniu_Region_Hosts));
    if (newRegionHosts == NULL)
    {
        free((void *)newHosts);
        return NULL;
    }
    newRegionHosts->hosts = (const char *const *)newHosts;

    for (size_t i = 0; i < hostsCount; i++)
    {
        if (hosts[i] == NULL)
        {
            break;
        }
        newHosts[i] = duplicate_host(hosts[i], useHttps);
        if (newHosts[i] == NULL)
        {
            for (size_t j = 0; j < i; j++)
            {
                free((void *)newHosts[j]);
            }
            free((void *)newHosts);
            free((void *)newRegionHosts);
            return NULL;
        }
        newRegionHosts->hostsCount = i + 1;
    }
    return newRegionHosts;
}

struct _Qiniu_Region_Hosts *_Qiniu_Region_Hosts_New_without_dup(const char *const *hosts, size_t hostsCount)
{
    struct _Qiniu_Region_Hosts *newRegionHosts = malloc(sizeof(struct _Qiniu_Region_Hosts));
    if (newRegionHosts == NULL)
    {
        return NULL;
    }
    newRegionHosts->hosts = hosts;
    newRegionHosts->hostsCount = hostsCount;
    return newRegionHosts;
}

void _Qiniu_Region_Hosts_Free(struct _Qiniu_Region_Hosts *hosts)
{
    if (hosts == NULL)
    {
        return;
    }
    for (int i = 0; i < hosts->hostsCount; i++)
    {
        free((void *)hosts->hosts[i]);
    }
    free((void *)hosts->hosts);
    hosts->hosts = NULL;
    hosts->hostsCount = 0;
    free((void *)hosts);
}

struct _Qiniu_Region_Service *_Qiniu_Region_Service_New(struct _Qiniu_Region_Hosts *preferredHosts, struct _Qiniu_Region_Hosts *alternativeHosts)
{
    struct _Qiniu_Region_Service *newService = malloc(sizeof(struct _Qiniu_Region_Service));
    if (newService == NULL)
    {
        return NULL;
    }
    newService->preferredHosts = preferredHosts;
    newService->alternativeHosts = alternativeHosts;

    return newService;
}

void _Qiniu_Region_Service_Free(struct _Qiniu_Region_Service *service)
{
    if (service == NULL)
    {
        return;
    }
    _Qiniu_Region_Hosts_Free(service->preferredHosts);
    service->preferredHosts = NULL;
    _Qiniu_Region_Hosts_Free(service->alternativeHosts);
    service->alternativeHosts = NULL;
    free(service);
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

void Qiniu_Region_Set_Up_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->upService == NULL)
    {
        region->upService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->upService->preferredHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->upService->preferredHosts);
        region->upService->preferredHosts = NULL;
    }
    region->upService->preferredHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Up_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->upService == NULL)
    {
        region->upService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->upService->alternativeHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->upService->alternativeHosts);
        region->upService->alternativeHosts = NULL;
    }
    region->upService->alternativeHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Io_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioService == NULL)
    {
        region->ioService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->ioService->preferredHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->ioService->preferredHosts);
        region->ioService->preferredHosts = NULL;
    }
    region->ioService->preferredHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Io_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioService == NULL)
    {
        region->ioService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->ioService->alternativeHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->ioService->alternativeHosts);
        region->ioService->alternativeHosts = NULL;
    }
    region->ioService->alternativeHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Io_Src_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioSrcService == NULL)
    {
        region->ioSrcService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->ioSrcService->preferredHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->ioSrcService->preferredHosts);
        region->ioSrcService->preferredHosts = NULL;
    }
    region->ioSrcService->preferredHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Io_Src_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ioSrcService == NULL)
    {
        region->ioSrcService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->ioSrcService->alternativeHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->ioSrcService->alternativeHosts);
        region->ioSrcService->alternativeHosts = NULL;
    }
    region->ioSrcService->alternativeHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Bucket_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ucService == NULL)
    {
        region->ucService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->ucService->preferredHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->ucService->preferredHosts);
        region->ucService->preferredHosts = NULL;
    }
    region->ucService->preferredHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Bucket_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->ucService == NULL)
    {
        region->ucService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->ucService->alternativeHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->ucService->alternativeHosts);
        region->ucService->alternativeHosts = NULL;
    }
    region->ucService->alternativeHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Rs_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsService == NULL)
    {
        region->rsService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->rsService->preferredHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->rsService->preferredHosts);
        region->rsService->preferredHosts = NULL;
    }
    region->rsService->preferredHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Rs_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsService == NULL)
    {
        region->rsService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->rsService->alternativeHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->rsService->alternativeHosts);
        region->rsService->alternativeHosts = NULL;
    }
    region->rsService->alternativeHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Rsf_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsfService == NULL)
    {
        region->rsfService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->rsfService->preferredHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->rsfService->preferredHosts);
        region->rsfService->preferredHosts = NULL;
    }
    region->rsfService->preferredHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Rsf_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->rsfService == NULL)
    {
        region->rsfService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->rsfService->alternativeHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->rsfService->alternativeHosts);
        region->rsfService->alternativeHosts = NULL;
    }
    region->rsfService->alternativeHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Api_Preferred_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->apiService == NULL)
    {
        region->apiService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->apiService->preferredHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->apiService->preferredHosts);
        region->apiService->preferredHosts = NULL;
    }
    region->apiService->preferredHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
}

void Qiniu_Region_Set_Api_Alternative_Hosts(Qiniu_Region *region, const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps)
{
    if (region == NULL)
    {
        return;
    }
    if (region->apiService == NULL)
    {
        region->apiService = _Qiniu_Region_Service_New(NULL, NULL);
    }
    if (region->apiService->alternativeHosts != NULL)
    {
        _Qiniu_Region_Hosts_Free(region->apiService->alternativeHosts);
        region->apiService->alternativeHosts = NULL;
    }
    region->apiService->alternativeHosts = _Qiniu_Region_Hosts_New(hosts, hostsCount, useHttps);
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
    if (region->upService->preferredHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->upService->preferredHosts->hostsCount;
    }
    return region->upService->preferredHosts->hosts;
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
    if (region->upService->alternativeHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->upService->alternativeHosts->hostsCount;
    }
    return region->upService->alternativeHosts->hosts;
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
    if (region->ioService->preferredHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioService->preferredHosts->hostsCount;
    }
    return region->ioService->preferredHosts->hosts;
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
    if (region->ioService->alternativeHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioService->alternativeHosts->hostsCount;
    }
    return region->ioService->alternativeHosts->hosts;
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
    if (region->ioSrcService->preferredHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioSrcService->preferredHosts->hostsCount;
    }
    return region->ioSrcService->preferredHosts->hosts;
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
    if (region->ioSrcService->alternativeHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ioSrcService->alternativeHosts->hostsCount;
    }
    return region->ioSrcService->alternativeHosts->hosts;
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
    if (region->ucService->preferredHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ucService->preferredHosts->hostsCount;
    }
    return region->ucService->preferredHosts->hosts;
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
    if (region->ucService->alternativeHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->ucService->alternativeHosts->hostsCount;
    }
    return region->ucService->alternativeHosts->hosts;
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
    if (region->rsService->preferredHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsService->preferredHosts->hostsCount;
    }
    return region->rsService->preferredHosts->hosts;
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
    if (region->rsService->alternativeHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsService->alternativeHosts->hostsCount;
    }
    return region->rsService->alternativeHosts->hosts;
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
    if (region->rsfService->preferredHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsfService->preferredHosts->hostsCount;
    }
    return region->rsfService->preferredHosts->hosts;
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
    if (region->rsfService->alternativeHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->rsfService->alternativeHosts->hostsCount;
    }
    return region->rsfService->alternativeHosts->hosts;
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
    if (region->apiService->preferredHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->apiService->preferredHosts->hostsCount;
    }
    return region->apiService->preferredHosts->hosts;
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
    if (region->apiService->alternativeHosts == NULL)
    {
        return NULL;
    }
    if (count != NULL)
    {
        *count = region->apiService->alternativeHosts->hostsCount;
    }
    return region->apiService->alternativeHosts->hosts;
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
        Qiniu_Region_Set_Up_Preferred_Hosts(region, (const char *const[]){"upload.qiniup.com", "up.qiniup.com"}, 2, useHttps);
        Qiniu_Region_Set_Up_Alternative_Hosts(region, (const char *const[]){"up.qbox.me"}, 1, useHttps);
        Qiniu_Region_Set_Io_Preferred_Hosts(region, (const char *const[]){"iovip.qiniuio.com"}, 1, useHttps);
        Qiniu_Region_Set_Io_Alternative_Hosts(region, (const char *const[]){"iovip.qbox.me"}, 1, useHttps);
    }
    else
    {
        Qiniu_snprintf(buf, sizeof(buf), "upload-%s.qiniup.com", regionId);
        Qiniu_snprintf(buf2, sizeof(buf2), "up-%s.qiniup.com", regionId);
        Qiniu_Region_Set_Up_Preferred_Hosts(region, (const char *const[]){buf, buf2}, 2, useHttps);
        Qiniu_snprintf(buf, sizeof(buf), "up-%s.qbox.me", regionId);
        Qiniu_Region_Set_Up_Alternative_Hosts(region, (const char *const[]){buf}, 1, useHttps);
        Qiniu_snprintf(buf, sizeof(buf), "iovip-%s.qiniuio.com", regionId);
        Qiniu_Region_Set_Io_Preferred_Hosts(region, (const char *const[]){buf}, 1, useHttps);
        Qiniu_snprintf(buf, sizeof(buf), "iovip-%s.qbox.me", regionId);
        Qiniu_Region_Set_Io_Alternative_Hosts(region, (const char *const[]){buf}, 1, useHttps);
    }
    Qiniu_snprintf(buf, sizeof(buf), "rs-%s.qiniuapi.com", regionId);
    Qiniu_Region_Set_Rs_Preferred_Hosts(region, (const char *const[]){buf}, 1, useHttps);
    Qiniu_snprintf(buf, sizeof(buf), "rs-%s.qbox.me", regionId);
    Qiniu_Region_Set_Rs_Alternative_Hosts(region, (const char *const[]){buf}, 1, useHttps);
    Qiniu_snprintf(buf, sizeof(buf), "rsf-%s.qiniuapi.com", regionId);
    Qiniu_Region_Set_Rsf_Preferred_Hosts(region, (const char *const[]){buf}, 1, useHttps);
    Qiniu_snprintf(buf, sizeof(buf), "rsf-%s.qbox.me", regionId);
    Qiniu_Region_Set_Rsf_Alternative_Hosts(region, (const char *const[]){buf}, 1, useHttps);
    Qiniu_snprintf(buf, sizeof(buf), "api-%s.qiniuapi.com", regionId);
    Qiniu_Region_Set_Api_Preferred_Hosts(region, (const char *const[]){buf}, 1, useHttps);
    Qiniu_snprintf(buf, sizeof(buf), "api-%s.qbox.me", regionId);
    Qiniu_Region_Set_Api_Alternative_Hosts(region, (const char *const[]){buf}, 1, useHttps);
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

static Qiniu_Error _Qiniu_Region_Query_call(Qiniu_Client *self, const char *accessKey, const char *const bucketName, cJSON **ret)
{
    char *url;
    Qiniu_Error err;
    Qiniu_Zero(err);
    const char *hosts[3] = {NULL, NULL, NULL};
    int hostsLen = 0;

    if (QINIU_UC_HOST != NULL)
    {
        hosts[hostsLen++] = QINIU_UC_HOST;
    }
    if (QINIU_UC_HOST_BACKUP != NULL)
    {
        hosts[hostsLen++] = QINIU_UC_HOST_BACKUP;
    }
    if (QINIU_API_HOST != NULL)
    {
        hosts[hostsLen++] = QINIU_API_HOST;
    }

    for (int i = 0; i < hostsLen; i++)
    {
        url = Qiniu_String_Concat(hosts[i], "/v4/query?ak=", accessKey, "&bucket=", bucketName, NULL);
        err = Qiniu_Client_Call(self, ret, url);
        Qiniu_Free(url);
        if (err.code == Qiniu_OK.code)
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
            return err;
        }
        else if (!isRegionQueryRetryable(err))
        {
            return err;
        }
    }
    if (err.code == 0)
    {
        err.code = 599;
        err.message = "no uc hosts available";
    }
    return err;
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
    if (err.code == Qiniu_OK.code)
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
            struct _Qiniu_Region_Hosts *upPreferredRegionHosts = NULL, *upAlternativeRegionHosts = NULL;
            cJSON *upDomainsJson = Qiniu_Json_GetObjectItem(upJson, "domains", NULL);
            if (upDomainsJson != NULL)
            {
                int upPreferredHostsCount = cJSON_GetArraySize(upDomainsJson);
                if (upPreferredHostsCount > 0)
                {
                    const char **upPreferredHosts = (const char **)malloc(sizeof(const char *) * upPreferredHostsCount);
                    for (int i = 0; i < upPreferredHostsCount; i++)
                    {
                        upPreferredHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(upDomainsJson, i, NULL), useHttps);
                    }
                    upPreferredRegionHosts = _Qiniu_Region_Hosts_New_without_dup(upPreferredHosts, upPreferredHostsCount);
                }
            }
            cJSON *upOldJson = Qiniu_Json_GetObjectItem(upJson, "old", NULL);
            if (upOldJson != NULL)
            {
                int upAlternativeHostsCount = cJSON_GetArraySize(upOldJson);
                if (upAlternativeHostsCount > 0)
                {
                    const char **upAlternativeHosts = (const char **)malloc(sizeof(const char *) * upAlternativeHostsCount);
                    for (int i = 0; i < upAlternativeHostsCount; i++)
                    {
                        upAlternativeHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(upOldJson, i, NULL), useHttps);
                    }
                    upAlternativeRegionHosts = _Qiniu_Region_Hosts_New_without_dup(upAlternativeHosts, upAlternativeHostsCount);
                }
            }
            struct _Qiniu_Region_Service *upService = _Qiniu_Region_Service_New(upPreferredRegionHosts, upAlternativeRegionHosts);
            region->upService = upService;
        }

        cJSON *ioJson = Qiniu_Json_GetObjectItem(firstHostJson, "io", NULL);
        if (ioJson != NULL)
        {
            struct _Qiniu_Region_Hosts *ioPreferredRegionHosts = NULL, *ioAlternativeRegionHosts = NULL;
            cJSON *ioDomainsJson = Qiniu_Json_GetObjectItem(ioJson, "domains", NULL);
            if (ioDomainsJson != NULL)
            {
                int ioPreferredHostsCount = cJSON_GetArraySize(ioDomainsJson);
                if (ioPreferredHostsCount > 0)
                {
                    const char **ioPreferredHosts = (const char **)malloc(sizeof(const char *) * ioPreferredHostsCount);
                    for (int i = 0; i < ioPreferredHostsCount; i++)
                    {
                        ioPreferredHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(ioDomainsJson, i, NULL), useHttps);
                    }
                    ioPreferredRegionHosts = _Qiniu_Region_Hosts_New_without_dup(ioPreferredHosts, ioPreferredHostsCount);
                }
            }
            cJSON *ioOldJson = Qiniu_Json_GetObjectItem(ioJson, "old", NULL);
            if (ioOldJson != NULL)
            {
                int ioAlternativeHostsCount = cJSON_GetArraySize(ioOldJson);
                if (ioAlternativeHostsCount > 0)
                {
                    const char **ioAlternativeHosts = (const char **)malloc(sizeof(const char *) * ioAlternativeHostsCount);
                    for (int i = 0; i < ioAlternativeHostsCount; i++)
                    {
                        ioAlternativeHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(ioOldJson, i, NULL), useHttps);
                    }
                    ioAlternativeRegionHosts = _Qiniu_Region_Hosts_New_without_dup(ioAlternativeHosts, ioAlternativeHostsCount);
                }
            }
            struct _Qiniu_Region_Service *ioService = _Qiniu_Region_Service_New(ioPreferredRegionHosts, ioAlternativeRegionHosts);
            region->ioService = ioService;
        }

        cJSON *ioSrcJson = Qiniu_Json_GetObjectItem(firstHostJson, "io_src", NULL);
        if (ioSrcJson != NULL)
        {
            struct _Qiniu_Region_Hosts *ioSrcPreferredRegionHosts = NULL, *ioSrcAlternativeRegionHosts = NULL;
            cJSON *ioSrcDomainsJson = Qiniu_Json_GetObjectItem(ioSrcJson, "domains", NULL);
            if (ioSrcDomainsJson != NULL)
            {
                int ioSrcPreferredHostsCount = cJSON_GetArraySize(ioSrcDomainsJson);
                if (ioSrcPreferredHostsCount > 0)
                {
                    const char **ioSrcPreferredHosts = (const char **)malloc(sizeof(const char *) * ioSrcPreferredHostsCount);
                    for (int i = 0; i < ioSrcPreferredHostsCount; i++)
                    {
                        ioSrcPreferredHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(ioSrcDomainsJson, i, NULL), useHttps);
                    }
                    ioSrcPreferredRegionHosts = _Qiniu_Region_Hosts_New_without_dup(ioSrcPreferredHosts, ioSrcPreferredHostsCount);
                }
            }
            cJSON *ioSrcOldJson = Qiniu_Json_GetObjectItem(ioSrcJson, "old", NULL);
            if (ioSrcOldJson != NULL)
            {
                int ioSrcAlternativeHostsCount = cJSON_GetArraySize(ioSrcOldJson);
                if (ioSrcAlternativeHostsCount > 0)
                {
                    const char **ioSrcAlternativeHosts = (const char **)malloc(sizeof(const char *) * ioSrcAlternativeHostsCount);
                    for (int i = 0; i < ioSrcAlternativeHostsCount; i++)
                    {
                        ioSrcAlternativeHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(ioSrcOldJson, i, NULL), useHttps);
                    }
                    ioSrcAlternativeRegionHosts = _Qiniu_Region_Hosts_New_without_dup(ioSrcAlternativeHosts, ioSrcAlternativeHostsCount);
                }
            }
            struct _Qiniu_Region_Service *ioSrcService = _Qiniu_Region_Service_New(ioSrcPreferredRegionHosts, ioSrcAlternativeRegionHosts);
            region->ioSrcService = ioSrcService;
        }

        cJSON *ucJson = Qiniu_Json_GetObjectItem(firstHostJson, "uc", NULL);
        if (ucJson != NULL)
        {
            struct _Qiniu_Region_Hosts *ucPreferredRegionHosts = NULL, *ucAlternativeRegionHosts = NULL;
            cJSON *ucDomainsJson = Qiniu_Json_GetObjectItem(ucJson, "domains", NULL);
            if (ucDomainsJson != NULL)
            {
                int ucPreferredHostsCount = cJSON_GetArraySize(ucDomainsJson);
                if (ucPreferredHostsCount > 0)
                {
                    const char **ucPreferredHosts = (const char **)malloc(sizeof(const char *) * ucPreferredHostsCount);
                    for (int i = 0; i < ucPreferredHostsCount; i++)
                    {
                        ucPreferredHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(ucDomainsJson, i, NULL), useHttps);
                    }
                    ucPreferredRegionHosts = _Qiniu_Region_Hosts_New_without_dup(ucPreferredHosts, ucPreferredHostsCount);
                }
            }
            cJSON *ucOldJson = Qiniu_Json_GetObjectItem(ucJson, "old", NULL);
            if (ucOldJson != NULL)
            {
                int ucAlternativeHostsCount = cJSON_GetArraySize(ucOldJson);
                if (ucAlternativeHostsCount > 0)
                {
                    const char **ucAlternativeHosts = (const char **)malloc(sizeof(const char *) * ucAlternativeHostsCount);
                    for (int i = 0; i < ucAlternativeHostsCount; i++)
                    {
                        ucAlternativeHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(ucOldJson, i, NULL), useHttps);
                    }
                    ucAlternativeRegionHosts = _Qiniu_Region_Hosts_New_without_dup(ucAlternativeHosts, ucAlternativeHostsCount);
                }
            }
            struct _Qiniu_Region_Service *ucService = _Qiniu_Region_Service_New(ucPreferredRegionHosts, ucAlternativeRegionHosts);
            region->ucService = ucService;
        }

        cJSON *rsJson = Qiniu_Json_GetObjectItem(firstHostJson, "rs", NULL);
        if (rsJson != NULL)
        {
            struct _Qiniu_Region_Hosts *rsPreferredRegionHosts = NULL, *rsAlternativeRegionHosts = NULL;
            cJSON *rsDomainsJson = Qiniu_Json_GetObjectItem(rsJson, "domains", NULL);
            if (rsDomainsJson != NULL)
            {
                int rsPreferredHostsCount = cJSON_GetArraySize(rsDomainsJson);
                if (rsPreferredHostsCount > 0)
                {
                    const char **rsPreferredHosts = (const char **)malloc(sizeof(const char *) * rsPreferredHostsCount);
                    for (int i = 0; i < rsPreferredHostsCount; i++)
                    {
                        rsPreferredHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(rsDomainsJson, i, NULL), useHttps);
                    }
                    rsPreferredRegionHosts = _Qiniu_Region_Hosts_New_without_dup(rsPreferredHosts, rsPreferredHostsCount);
                }
            }
            cJSON *rsOldJson = Qiniu_Json_GetObjectItem(rsJson, "old", NULL);
            if (rsOldJson != NULL)
            {
                int rsAlternativeHostsCount = cJSON_GetArraySize(rsOldJson);
                if (rsAlternativeHostsCount > 0)
                {
                    const char **rsAlternativeHosts = (const char **)malloc(sizeof(const char *) * rsAlternativeHostsCount);
                    for (int i = 0; i < rsAlternativeHostsCount; i++)
                    {
                        rsAlternativeHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(rsOldJson, i, NULL), useHttps);
                    }
                    rsAlternativeRegionHosts = _Qiniu_Region_Hosts_New_without_dup(rsAlternativeHosts, rsAlternativeHostsCount);
                }
            }
            struct _Qiniu_Region_Service *rsService = _Qiniu_Region_Service_New(rsPreferredRegionHosts, rsAlternativeRegionHosts);
            region->rsService = rsService;
        }

        cJSON *rsfJson = Qiniu_Json_GetObjectItem(firstHostJson, "rsf", NULL);
        if (rsfJson != NULL)
        {
            struct _Qiniu_Region_Hosts *rsfPreferredRegionHosts = NULL, *rsfAlternativeRegionHosts = NULL;
            cJSON *rsfDomainsJson = Qiniu_Json_GetObjectItem(rsfJson, "domains", NULL);
            if (rsfDomainsJson != NULL)
            {
                int rsfPreferredHostsCount = cJSON_GetArraySize(rsfDomainsJson);
                if (rsfPreferredHostsCount > 0)
                {
                    const char **rsfPreferredHosts = (const char **)malloc(sizeof(const char *) * rsfPreferredHostsCount);
                    for (int i = 0; i < rsfPreferredHostsCount; i++)
                    {
                        rsfPreferredHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(rsfDomainsJson, i, NULL), useHttps);
                    }
                    rsfPreferredRegionHosts = _Qiniu_Region_Hosts_New_without_dup(rsfPreferredHosts, rsfPreferredHostsCount);
                }
            }
            cJSON *rsfOldJson = Qiniu_Json_GetObjectItem(rsfJson, "old", NULL);
            if (rsfOldJson != NULL)
            {
                int rsfAlternativeHostsCount = cJSON_GetArraySize(rsfOldJson);
                if (rsfAlternativeHostsCount > 0)
                {
                    const char **rsfAlternativeHosts = (const char **)malloc(sizeof(const char *) * rsfAlternativeHostsCount);
                    for (int i = 0; i < rsfAlternativeHostsCount; i++)
                    {
                        rsfAlternativeHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(rsfOldJson, i, NULL), useHttps);
                    }
                    rsfAlternativeRegionHosts = _Qiniu_Region_Hosts_New_without_dup(rsfAlternativeHosts, rsfAlternativeHostsCount);
                }
            }
            struct _Qiniu_Region_Service *rsfService = _Qiniu_Region_Service_New(rsfPreferredRegionHosts, rsfAlternativeRegionHosts);
            region->rsfService = rsfService;
        }

        cJSON *apiJson = Qiniu_Json_GetObjectItem(firstHostJson, "api", NULL);
        if (apiJson != NULL)
        {
            struct _Qiniu_Region_Hosts *apiPreferredRegionHosts = NULL, *apiAlternativeRegionHosts = NULL;
            cJSON *apiDomainsJson = Qiniu_Json_GetObjectItem(apiJson, "domains", NULL);
            if (apiDomainsJson != NULL)
            {
                int apiPreferredHostsCount = cJSON_GetArraySize(apiDomainsJson);
                if (apiPreferredHostsCount > 0)
                {
                    const char **apiPreferredHosts = (const char **)malloc(sizeof(const char *) * apiPreferredHostsCount);
                    for (int i = 0; i < apiPreferredHostsCount; i++)
                    {
                        apiPreferredHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(apiDomainsJson, i, NULL), useHttps);
                    }
                    apiPreferredRegionHosts = _Qiniu_Region_Hosts_New_without_dup(apiPreferredHosts, apiPreferredHostsCount);
                }
            }
            cJSON *apiOldJson = Qiniu_Json_GetObjectItem(apiJson, "old", NULL);
            if (apiOldJson != NULL)
            {
                int apiAlternativeHostsCount = cJSON_GetArraySize(apiOldJson);
                if (apiAlternativeHostsCount > 0)
                {
                    const char **apiAlternativeHosts = (const char **)malloc(sizeof(const char *) * apiAlternativeHostsCount);
                    for (int i = 0; i < apiAlternativeHostsCount; i++)
                    {
                        apiAlternativeHosts[i] = duplicate_host(Qiniu_Json_GetStringAt(apiOldJson, i, NULL), useHttps);
                    }
                    apiAlternativeRegionHosts = _Qiniu_Region_Hosts_New_without_dup(apiAlternativeHosts, apiAlternativeHostsCount);
                }
            }
            struct _Qiniu_Region_Service *apiService = _Qiniu_Region_Service_New(apiPreferredRegionHosts, apiAlternativeRegionHosts);
            region->apiService = apiService;
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

struct Qiniu_Region_Cache
{
    Qiniu_Region *region;
    const char *accessKey, *bucketName, *ucHost, *ucHostBackup, *apiHost;
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

static int
_Qiniu_Region_Cache_Compare(const void *a, const void *b, void *user_data)
{
    int result;
    struct Qiniu_Region_Cache *cacheA = (struct Qiniu_Region_Cache *)a;
    struct Qiniu_Region_Cache *cacheB = (struct Qiniu_Region_Cache *)b;
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
    result = _Qiniu_Compare_Str(cacheA->ucHost, cacheB->ucHost);
    if (!result)
    {
        return result;
    }
    result = _Qiniu_Compare_Str(cacheA->ucHostBackup, cacheB->ucHostBackup);
    if (!result)
    {
        return result;
    }
    result = _Qiniu_Compare_Str(cacheA->apiHost, cacheB->apiHost);
    if (!result)
    {
        return result;
    }
}

static uint64_t _Qiniu_Region_Cache_Hash(const void *r, uint64_t seed0, uint64_t seed1)
{
    struct Qiniu_Region_Cache *cache = (struct Qiniu_Region_Cache *)r;
    uint64_t hash = 0;
    if (cache->accessKey != NULL)
    {
        hash ^= hashmap_sip(cache->accessKey, strlen(cache->accessKey), seed0, seed1);
    }
    if (cache->bucketName != NULL)
    {
        hash ^= hashmap_sip(cache->bucketName, strlen(cache->bucketName), seed0, seed1);
    }
    if (cache->ucHost != NULL)
    {
        hash ^= hashmap_sip(cache->ucHost, strlen(cache->ucHost), seed0, seed1);
    }
    if (cache->ucHostBackup != NULL)
    {
        hash ^= hashmap_sip(cache->ucHostBackup, strlen(cache->ucHostBackup), seed0, seed1);
    }
    if (cache->apiHost != NULL)
    {
        hash ^= hashmap_sip(cache->apiHost, strlen(cache->apiHost), seed0, seed1);
    }
    return hash;
}

static void _Qiniu_Region_Cache_Free(void *r)
{
    struct Qiniu_Region_Cache *cache = (struct Qiniu_Region_Cache *)r;
    Qiniu_Free((void *)cache->accessKey);
    Qiniu_Free((void *)cache->bucketName);
    Qiniu_Free((void *)cache->ucHost);
    Qiniu_Free((void *)cache->ucHostBackup);
    Qiniu_Free((void *)cache->apiHost);
    Qiniu_Region_Free(cache->region);
    Qiniu_Zero_Ptr(cache);
}

static Qiniu_Error _Qiniu_Region_Auto_Query_With_Cache(Qiniu_Client *self, const char *accessKey, const char *bucketName, Qiniu_Region **foundRegion)
{
    if (accessKey == NULL)
    {
        accessKey = QINIU_ACCESS_KEY;
    }
    const struct Qiniu_Region_Cache cacheKey = {
        .accessKey = accessKey,
        .bucketName = bucketName,
        .ucHost = QINIU_UC_HOST,
        .ucHostBackup = QINIU_UC_HOST_BACKUP,
        .apiHost = QINIU_API_HOST,
    };
    struct Qiniu_Region_Cache *cache = NULL;
    if (self->cachedRegions != NULL)
    {
        cache = (struct Qiniu_Region_Cache *)hashmap_get(self->cachedRegions, &cacheKey);
        if (cache != NULL && !Qiniu_Region_Is_Expired(cache->region))
        {
            *foundRegion = cache->region;
            return Qiniu_OK;
        }
    }
    Qiniu_Error err = _Qiniu_Region_Query(self, foundRegion, accessKey, bucketName, self->autoQueryHttpsRegion);
    if (err.code != Qiniu_OK.code)
    {
        if (cache != NULL)
        { // 有已经过期的缓存区域可以使用
            *foundRegion = cache->region;
            return Qiniu_OK;
        }
        return err;
    }
    if (self->cachedRegions == NULL)
    {
        self->cachedRegions = hashmap_new(sizeof(struct Qiniu_Region_Cache), 0, rand(), rand(), _Qiniu_Region_Cache_Hash, _Qiniu_Region_Cache_Compare, _Qiniu_Region_Cache_Free, NULL);
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
            struct Qiniu_Region_Cache *newCache = malloc(sizeof(struct Qiniu_Region_Cache));
            if (newCache != NULL)
            {
                *newCache = (struct Qiniu_Region_Cache){
                    .region = *foundRegion,
                    .accessKey = Qiniu_String_Dup(accessKey),
                    .bucketName = Qiniu_String_Dup(bucketName),
                    .ucHost = Qiniu_String_Dup(QINIU_UC_HOST),
                    .ucHostBackup = Qiniu_String_Dup(QINIU_UC_HOST_BACKUP),
                    .apiHost = Qiniu_String_Dup(QINIU_API_HOST),
                };
                hashmap_set(self->cachedRegions, newCache);
            }
        }
    }
    return Qiniu_OK;
}

Qiniu_Error _Qiniu_Region_Get_Up_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == Qiniu_OK.code)
        {
            goto foundCache;
        }
        *host = QINIU_UP_HOST;
        return err;
    }
foundCache:
    hosts = Qiniu_Region_Get_Up_Preferred_Hosts(foundRegion, &count);
    if (count == 0)
    {
        *host = QINIU_UP_HOST;
    }
    else
    {
        *host = hosts[0];
    }
    return err;
}

Qiniu_Error _Qiniu_Region_Get_Io_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == Qiniu_OK.code)
        {
            goto foundCache;
        }
        *host = QINIU_UP_HOST;
        return err;
    }
foundCache:
    hosts = Qiniu_Region_Get_Io_Preferred_Hosts(foundRegion, &count);
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

Qiniu_Error _Qiniu_Region_Get_Rs_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == Qiniu_OK.code)
        {
            goto foundCache;
        }
        *host = QINIU_UP_HOST;
        return err;
    }
foundCache:
    hosts = Qiniu_Region_Get_Rs_Preferred_Hosts(foundRegion, &count);
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

Qiniu_Error _Qiniu_Region_Get_Rsf_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == Qiniu_OK.code)
        {
            goto foundCache;
        }
        *host = QINIU_UP_HOST;
        return err;
    }
foundCache:
    hosts = Qiniu_Region_Get_Rsf_Preferred_Hosts(foundRegion, &count);
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

Qiniu_Error _Qiniu_Region_Get_Api_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host)
{
    const char *const *hosts;
    size_t count;
    Qiniu_Error err = Qiniu_OK;
    Qiniu_Region *foundRegion = NULL;

    if (self->specifiedRegion)
    {
        foundRegion = self->specifiedRegion;
        goto foundCache;
    }
    else if (self->autoQueryRegion)
    {
        err = _Qiniu_Region_Auto_Query_With_Cache(self, accessKey, bucketName, &foundRegion);
        if (err.code == Qiniu_OK.code)
        {
            goto foundCache;
        }
        *host = QINIU_UP_HOST;
        return err;
    }
foundCache:
    hosts = Qiniu_Region_Get_Api_Preferred_Hosts(foundRegion, &count);
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
