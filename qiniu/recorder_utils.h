/*
 ============================================================================
 Name        : recorder_utils.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_RECORDER_UTILS_H
#define QINIU_RECORDER_UTILS_H

#include "base.h"
#include "recorder.h"

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	/*============================================================================*/

	QINIU_DLLAPI extern Qiniu_Bool Qiniu_Utils_Extract_Bucket(const char *uptoken, const char **pAccessKey, const char **pBucket);
	QINIU_DLLAPI extern Qiniu_Error Qiniu_Utils_Generate_RecorderKey(const char *uptoken, const char *version, const char *key, const char *localFile, const char **pRecorderKey);
	QINIU_DLLAPI extern Qiniu_Error Qiniu_Utils_Find_Medium(Qiniu_Recorder *recorder, const char *recorderKey, Qiniu_Int64 expectedVersion, struct Qiniu_Record_Medium *medium, Qiniu_FileInfo *fileInfo, Qiniu_Bool *ok);
	QINIU_DLLAPI extern Qiniu_Error Qiniu_Utils_New_Medium(Qiniu_Recorder *recorder, const char *recorderKey, Qiniu_Int64 version, struct Qiniu_Record_Medium *medium, Qiniu_FileInfo *fileInfo);

	/*============================================================================*/

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif // QINIU_RECORDER_UTILS_H
