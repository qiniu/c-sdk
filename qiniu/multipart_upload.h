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
        const char *upHost; //it's better move upHost to client
        Qiniu_Int64 partSize;
        const char *mimeType;
        int tryTimes;
        // void *notifyRecvr;
        // Qiniu_Rio_FnNotify notify;
        // Qiniu_Rio_FnNotifyErr notifyErr;
        // Qiniu_Rio_BlkputRet* progresses;
        // size_t blockCnt;
        // Qiniu_Rio_ThreadModel threadModel;

        // For those file systems that save file name as Unicode strings,
        // use this field to name the local file name in UTF-8 format for CURL.
        const char *localFileName;

        // For those who want to invoke a upload callback on the business server
        // which returns a JSON object.
        void *callbackRet;
        Qiniu_Error (*callbackRetParser)(void *, Qiniu_Json *);

        // Use xVarsList to pass user defined variables and xVarsCount to pass the count of them.
        //
        // (extra->xVarsList[i])[0] set as the variable name, e.g. "x:Price".
        // **NOTICE**: User defined variable's name MUST starts with a prefix string "x:".
        //
        // (extra->xVarsList[i])[1] set as the value, e.g. "priceless".
        // const char *(*xVarsList)[2];
        // int xVarsCount;

    } Qiniu_Multipart_PutExtra;

    typedef struct
    {
        char *uploadId;
    } Qiniu_InitPart_Ret;
    typedef struct
    {
        struct
        {
            char *etag;
            int partNum;
        } * PartEtag;

        int totalPartNum;

    } Qiniu_UploadParts_Ret;
    typedef struct
    {
        char *hash;
        char *key;
    } Qiniu_CompleteUpload_Ret;

    QINIU_DLLAPI extern Qiniu_Error
    Qiniu_Multipart_PutWithKey(
        Qiniu_Client *client, Qiniu_CompleteUpload_Ret *ret,
        const char *uptoken, const char *key, const char *localFile, Qiniu_Multipart_PutExtra *extraParam);

    /*============================================================================*/

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // QINIU_MULTIPART_UPLOAD_H