#ifndef QINIU_PRIVATE_REGION_H
#define QINIU_PRIVATE_REGION_H
#include <stddef.h>
#include "../http.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct _Qiniu_Region_Hosts
    {
        const char *const *hosts;
        size_t hostsCount;
    };

    struct _Qiniu_Region_Service
    {
        struct _Qiniu_Region_Hosts *preferredHosts;
        struct _Qiniu_Region_Hosts *alternativeHosts;
    };

    struct _Qiniu_Region
    {
        const char *regionId;
        Qiniu_Uint64 expirationTime;

        struct _Qiniu_Region_Service *upService;
        struct _Qiniu_Region_Service *ioService;
        struct _Qiniu_Region_Service *ioSrcService;
        struct _Qiniu_Region_Service *ucService;
        struct _Qiniu_Region_Service *rsService;
        struct _Qiniu_Region_Service *rsfService;
        struct _Qiniu_Region_Service *apiService;
    };

    static struct _Qiniu_Region_Hosts *_Qiniu_Region_Hosts_New(const char *const *hosts, size_t hostsCount, Qiniu_Bool useHttps);
    static struct _Qiniu_Region_Hosts *_Qiniu_Region_Hosts_New_without_dup(const char *const *hosts, size_t hostsCount);
    static void _Qiniu_Region_Hosts_Free(struct _Qiniu_Region_Hosts *hosts);

    static struct _Qiniu_Region_Service *_Qiniu_Region_Service_New(struct _Qiniu_Region_Hosts *preferredHosts, struct _Qiniu_Region_Hosts *alternativeHosts);
    static void _Qiniu_Region_Service_Free(struct _Qiniu_Region_Service *service);

    Qiniu_Error _Qiniu_Region_Get_Up_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Get_Io_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Get_Rs_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Get_Rsf_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Get_Api_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Query(Qiniu_Client *self, struct _Qiniu_Region **region, const char *accessKey, const char *const bucketName, Qiniu_Bool useHttps);

#ifdef __cplusplus
}
#endif

#endif // QINIU_PRIVATE_REGION_H
