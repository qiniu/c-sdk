/*
 ============================================================================
 Name        : recorder.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_RECORDER_H
#define QINIU_RECORDER_H

#include "base.h"
#define Qiniu_Recorder_Read_Error 9801
#define Qiniu_Recorder_Write_Error 9802

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /*============================================================================*/
    struct Qiniu_Record_Medium;
    struct Qiniu_Recorder;

    typedef struct Qiniu_Record_Medium
    {
        Qiniu_Error (*readEntry)(const struct Qiniu_Record_Medium *medium, char *dest, size_t toRead, size_t *haveRead);
        Qiniu_Error (*hasNextEntry)(const struct Qiniu_Record_Medium *medium, Qiniu_Bool *has);
        Qiniu_Error (*writeEntry)(const struct Qiniu_Record_Medium *medium, const char *src, size_t *written);
        Qiniu_Error (*close)(const struct Qiniu_Record_Medium *medium);
        void *data;
    } Qiniu_Record_Medium;

    typedef struct Qiniu_Recorder
    {
        Qiniu_Error (*open)(struct Qiniu_Recorder *recorder, const char *id, const char *mode, struct Qiniu_Record_Medium *medium);
        Qiniu_Error (*remove)(struct Qiniu_Recorder *recorder, const char *id);
        void (*free)(struct Qiniu_Recorder *recorder);
        void *data;
    } Qiniu_Recorder;

    QINIU_DLLAPI extern Qiniu_Error Qiniu_FileSystem_Recorder_New(const char *rootPath, struct Qiniu_Recorder *recorder);
    /*============================================================================*/

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif // QINIU_RECORDER_H
