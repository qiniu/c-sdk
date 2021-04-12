/*
 ============================================================================
 Name        : multipart_upload.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_MULTIPART_UPLOAD_H
#define QINIU_MULTIPART_UPLOAD_H

#include "http.h"
#include "io.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

    /*============================================================================*/

    typedef struct _Qiniu_Multipart_PutExtra
    {
        const char *upHost;   //if not set explicitly ,will use global QINIU_UP_HOST;
        Qiniu_Int64 partSize; //size for each part
        const char *mimeType;
        int tryTimes;
        Qiniu_Bool enableContentMd5; //calulate md5  and set to request.header["Content-MD5"]
    } Qiniu_Multipart_PutExtra;

    typedef struct
    {
        char *hash;
        char *key;
    } Qiniu_MultipartUpload_Result;

    /*
    func: Qiniu_Multipart_PutFile
    input: it's allowed(but not recommend) to set key="" ,which means the keyname is empty string,
            if key=NULL means the keyname is determined by server;
    note: the client not support concurrent usage , detail refer to: https://developer.qiniu.com/kodo/sdk/cpp             
    */
    QINIU_DLLAPI extern Qiniu_Error Qiniu_Multipart_PutFile(
        Qiniu_Client *client, const char *uptoken, const char *key,
        const char *localFile, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *ret);

    /*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // QINIU_MULTIPART_UPLOAD_H