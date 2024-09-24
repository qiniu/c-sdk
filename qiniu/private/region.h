#ifndef QINIU_PRIVATE_REGION_H
#define QINIU_PRIVATE_REGION_H
#include <stddef.h>
#include "../http.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct _Qiniu_Region_Service
    {
        const char *const *hosts;
        size_t acceleratedHostsCount, preferredHostsCount, alternativeHostsCount;
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

    struct _Qiniu_Region_Service *_Qiniu_Region_Service_New(const char *const *acceleratedHosts, size_t acceleratedHostsCount,
                                                            const char *const *preferredHosts, size_t preferredHostsCount,
                                                            const char *const *alternativeHosts, size_t alternativeHostsCount,
                                                            Qiniu_Bool useHttps);
    void _Qiniu_Region_Service_Free(struct _Qiniu_Region_Service *service);

    Qiniu_Error _Qiniu_Region_Get_Io_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Get_Rs_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Get_Rsf_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);
    Qiniu_Error _Qiniu_Region_Get_Api_Host(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char **host);

    Qiniu_Error _Qiniu_Region_Get_Up_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count);
    Qiniu_Error _Qiniu_Region_Get_Io_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count);
    Qiniu_Error _Qiniu_Region_Get_Rs_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count);
    Qiniu_Error _Qiniu_Region_Get_Rsf_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count);
    Qiniu_Error _Qiniu_Region_Get_Api_Hosts(Qiniu_Client *self, const char *accessKey, const char *bucketName, const char *const **hosts, size_t *count);
    Qiniu_Error _Qiniu_Region_Query(Qiniu_Client *self, struct _Qiniu_Region **region, const char *accessKey, const char *const bucketName, Qiniu_Bool useHttps);

    const char *const *_Qiniu_Region_Get_All_Up_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled);
    const char *const *_Qiniu_Region_Get_All_Io_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled);
    const char *const *_Qiniu_Region_Get_All_Io_Src_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled);
    const char *const *_Qiniu_Region_Get_All_Bucket_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled);
    const char *const *_Qiniu_Region_Get_All_Rs_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled);
    const char *const *_Qiniu_Region_Get_All_Rsf_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled);
    const char *const *_Qiniu_Region_Get_All_Api_Hosts(Qiniu_Region *region, size_t *count, Qiniu_Bool accelerationEnabled);
#ifdef __cplusplus
}
#endif

#endif // QINIU_PRIVATE_REGION_H
