/*
 ============================================================================
 Name        : conf.h
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#ifndef QINIU_CONF_H
#define QINIU_CONF_H

#include "macro.h"
#include "base.h"

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	QINIU_DLLAPI extern const char *version;
	QINIU_DLLAPI extern const char *QINIU_ACCESS_KEY;
	QINIU_DLLAPI extern const char *QINIU_SECRET_KEY;

	// 公共服务http域名
	//
	// 不再推荐使用，建议使用 Qiniu_Region 结构体替代
	QINIU_DLLAPI extern const char *QINIU_RS_HOST;
	QINIU_DLLAPI extern const char *QINIU_RSF_HOST;
	QINIU_DLLAPI extern const char *QINIU_IOVIP_HOST;
	QINIU_DLLAPI extern const char *QINIU_UP_HOST;
	QINIU_DLLAPI extern const char *QINIU_UC_HOST;
	QINIU_DLLAPI extern const char *QINIU_UC_HOST_BACKUP;
	QINIU_DLLAPI extern const char *QINIU_API_HOST;
	QINIU_DLLAPI extern const char *QINIU_FUSION_HOST;

	// 设置华东机房域名
	//
	// 不再推荐使用，建议使用 Qiniu_Use_Region("z0") 方法替代
	QINIU_DLLAPI extern void Qiniu_Use_Zone_Huadong(Qiniu_Bool useHttps);

	// 设置华北机房域名
	//
	// 不再推荐使用，建议使用 Qiniu_Use_Region("z1") 方法替代
	QINIU_DLLAPI extern void Qiniu_Use_Zone_Huabei(Qiniu_Bool useHttps);

	// 设置华南机房域名
	//
	// 不再推荐使用，建议使用 Qiniu_Use_Region("z2") 方法替代
	QINIU_DLLAPI extern void Qiniu_Use_Zone_Huanan(Qiniu_Bool useHttps);

	// 设置北美机房域名
	//
	// 不再推荐使用，建议使用 Qiniu_Use_Region("na0") 方法替代
	QINIU_DLLAPI extern void Qiniu_Use_Zone_Beimei(Qiniu_Bool useHttps);

	// 设置新加坡机房
	//
	// 不再推荐使用，建议使用 Qiniu_Use_Region("as0") 方法替代
	QINIU_DLLAPI extern void Qiniu_Use_Zone_Dongnanya(Qiniu_Bool useHttps);

	// 设置华东二区机房
	//
	// 不再推荐使用，建议使用 Qiniu_Use_Region("cn-east-2") 方法替代
	QINIU_DLLAPI extern void Qiniu_Use_Zone_Cn_East_2(Qiniu_Bool useHttps);

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif
