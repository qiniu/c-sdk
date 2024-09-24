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
#include "recorder.h"

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /*============================================================================*/
    typedef struct
    {
        char *md5;
        char *etag;
        int partNum; //Attention: partNum start with 1
    } Qiniu_UploadPartResp;

    typedef void (*NotifyFunc)(Qiniu_UploadPartResp *partResp);
    typedef void (*NotifyErrFunc)(int partNum, Qiniu_Error err);

    typedef struct _Qiniu_Multipart_PutExtra
    {
        const char *upHost;   // if not set explicitly, will use global upHosts;
        Qiniu_Int64 partSize; //size for each part
        const char *mimeType;
        int tryTimes;                //at least 1, default=3
        Qiniu_Bool enableContentMd5; //calulate md5  and set to request.header["Content-MD5"]
        NotifyFunc notify;
        NotifyErrFunc notifyErr;

        // Use xVarsList to pass user defined variables and xVarsCount to pass the count of them.
        // (extra->xVarsList[i])[0] set as the name, e.g. "x:Price", MUST starts with "x:".
        // (extra->xVarsList[i])[1] set as the value, e.g. "fiveRMB".
        const char *(*xVarsList)[2];
        int xVarsCount;

        //(extra->metaList[i])[0] :set metakey
        //(extra->metaList[i])[1] :set metavalue
        const char *(*metaList)[2];
        int metaCount;

        Qiniu_Recorder *recorder;

        // Specify multiple upHosts, if not set explicitly, will global QINIU_UP_HOST
        const char *const *upHosts;
        size_t upHostsCount;

        // Uploading file progress
        void (*uploadingProgress)(size_t ultotal, size_t ulnow);
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
        const char *fileName, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *uploadResult);

    QINIU_DLLAPI extern Qiniu_Error Qiniu_Multipart_Put(
        Qiniu_Client *client, const char *uptoken, const char *key,
        Qiniu_ReaderAt reader, Qiniu_Int64 fsize, Qiniu_Multipart_PutExtra *extraParam, Qiniu_MultipartUpload_Result *uploadResult);
    /*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // QINIU_MULTIPART_UPLOAD_H
