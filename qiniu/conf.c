/*
 ============================================================================
 Name        : conf.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include "conf.h"

const char *version = "7.9.0";

/*============================================================================*/

const char *QINIU_ACCESS_KEY = "<Please apply your access key>";
const char *QINIU_SECRET_KEY = "<Dont send your secret key to anyone>";

// 公共服务http域名
//
// 不再推荐使用，建议使用 Qiniu_Region 结构体替代
const char *QINIU_RS_HOST = "http://rs.qbox.me";
const char *QINIU_RSF_HOST = "http://rsf.qbox.me";
const char *QINIU_API_HOST = "http://api.qiniu.com";
const char *QINIU_FUSION_HOST = "http://fusion.qiniuapi.com";
const char *QINIU_UC_HOST = "http://uc.qiniuapi.com";
const char *QINIU_UC_HOST_BACKUP = "http://kodo-config.qiniuapi.com";
const char *QINIU_UC_HOST_BACKUP_2 = "http://uc.qbox.me";

// 默认华东机房
//
// 不再推荐使用，建议使用 Qiniu_Region 结构体替代
const char *QINIU_UP_HOST = "http://upload.qiniup.com";
const char *QINIU_IOVIP_HOST = "http://iovip.qbox.me";

// 设置华东机房域名
//
// 不再推荐使用，建议使用 Qiniu_Use_Region("z0") 方法替代
void Qiniu_Use_Zone_Huadong(Qiniu_Bool useHttps)
{
    if (useHttps == Qiniu_True)
    {
        QINIU_RS_HOST = "https://rs.qbox.me";
        QINIU_RSF_HOST = "https://rsf.qbox.me";
        QINIU_API_HOST = "https://api.qiniu.com";
        QINIU_UP_HOST = "https://upload.qiniup.com";
        QINIU_IOVIP_HOST = "https://iovip.qbox.me";
    }
    else
    {
        QINIU_RS_HOST = "http://rs.qbox.me";
        QINIU_RSF_HOST = "http://rsf.qbox.me";
        QINIU_API_HOST = "http://api.qiniu.com";
        QINIU_UP_HOST = "http://upload.qiniup.com";
        QINIU_IOVIP_HOST = "http://iovip.qbox.me";
    }
}

// 设置华北机房域名
//
// 不再推荐使用，建议使用 Qiniu_Use_Region("z1") 方法替代
void Qiniu_Use_Zone_Huabei(Qiniu_Bool useHttps)
{
    if (useHttps == Qiniu_True)
    {
        QINIU_RS_HOST = "https://rs-z1.qbox.me";
        QINIU_RSF_HOST = "https://rsf-z1.qbox.me";
        QINIU_API_HOST = "https://api-z1.qiniu.com";
        QINIU_UP_HOST = "https://upload-z1.qiniup.com";
        QINIU_IOVIP_HOST = "https://iovip-z1.qbox.me";
    }
    else
    {
        QINIU_RS_HOST = "http://rs-z1.qbox.me";
        QINIU_RSF_HOST = "http://rsf-z1.qbox.me";
        QINIU_API_HOST = "http://api-z1.qiniu.com";
        QINIU_UP_HOST = "http://upload-z1.qiniup.com";
        QINIU_IOVIP_HOST = "http://iovip-z1.qbox.me";
    }
}

// 设置华南机房域名
//
// 不再推荐使用，建议使用 Qiniu_Use_Region("z2") 方法替代
void Qiniu_Use_Zone_Huanan(Qiniu_Bool useHttps)
{
    if (useHttps == Qiniu_True)
    {
        QINIU_RS_HOST = "https://rs-z2.qbox.me";
        QINIU_RSF_HOST = "https://rsf-z2.qbox.me";
        QINIU_API_HOST = "https://api-z2.qiniu.com";
        QINIU_UP_HOST = "https://upload-z2.qiniup.com";
        QINIU_IOVIP_HOST = "https://iovip-z2.qbox.me";
    }
    else
    {
        QINIU_RS_HOST = "http://rs-z2.qbox.me";
        QINIU_RSF_HOST = "http://rsf-z2.qbox.me";
        QINIU_API_HOST = "http://api-z2.qiniu.com";
        QINIU_UP_HOST = "http://upload-z2.qiniup.com";
        QINIU_IOVIP_HOST = "http://iovip-z2.qbox.me";
    }
}

// 设置北美机房域名
//
// 不再推荐使用，建议使用 Qiniu_Use_Region("na0") 方法替代
void Qiniu_Use_Zone_Beimei(Qiniu_Bool useHttps)
{
    if (useHttps == Qiniu_True)
    {
        QINIU_RS_HOST = "https://rs-na0.qbox.me";
        QINIU_RSF_HOST = "https://rsf-na0.qbox.me";
        QINIU_API_HOST = "https://api-na0.qiniu.com";
        QINIU_UP_HOST = "https://upload-na0.qiniup.com";
        QINIU_IOVIP_HOST = "https://iovip-na0.qbox.me";
    }
    else
    {
        QINIU_RS_HOST = "http://rs-na0.qbox.me";
        QINIU_RSF_HOST = "http://rsf-na0.qbox.me";
        QINIU_API_HOST = "http://api-na0.qiniu.com";
        QINIU_UP_HOST = "http://upload-na0.qiniup.com";
        QINIU_IOVIP_HOST = "http://iovip-na0.qbox.me";
    }
}

// 设置新加坡机房
//
// 不再推荐使用，建议使用 Qiniu_Use_Region("as0") 方法替代
void Qiniu_Use_Zone_Dongnanya(Qiniu_Bool useHttps)
{
    if (useHttps == Qiniu_True)
    {
        QINIU_RS_HOST = "https://rs-as0.qbox.me";
        QINIU_RSF_HOST = "https://rsf-as0.qbox.me";
        QINIU_API_HOST = "https://api-as0.qiniu.com";
        QINIU_UP_HOST = "https://upload-as0.qiniup.com";
        QINIU_IOVIP_HOST = "https://iovip-as0.qbox.me";
    }
    else
    {
        QINIU_RS_HOST = "http://rs-as0.qbox.me";
        QINIU_RSF_HOST = "http://rsf-as0.qbox.me";
        QINIU_API_HOST = "http://api-as0.qiniu.com";
        QINIU_UP_HOST = "http://upload-as0.qiniup.com";
        QINIU_IOVIP_HOST = "http://iovip-as0.qbox.me";
    }
}

// 设置华东二区机房
//
// 不再推荐使用，建议使用 Qiniu_Use_Region("cn-east-2") 方法替代
void Qiniu_Use_Zone_Cn_East_2(Qiniu_Bool useHttps)
{
    if (useHttps == Qiniu_True)
    {
        QINIU_RS_HOST = "https://rs-cn-east-2.qiniuapi.com";
        QINIU_RSF_HOST = "https://rsf-cn-east-2.qiniuapi.com";
        QINIU_API_HOST = "https://api-cn-east-2.qiniuapi.com";
        QINIU_UP_HOST = "https://upload-cn-east-2.qiniup.com";
        QINIU_IOVIP_HOST = "https://iovip-cn-east-2.qiniuio.com";
    }
    else
    {
        QINIU_RS_HOST = "http://rs-cn-east-2.qiniuapi.com";
        QINIU_RSF_HOST = "http://rsf-cn-east-2.qiniuapi.com";
        QINIU_API_HOST = "http://api-cn-east-2.qiniuapi.com";
        QINIU_UP_HOST = "http://upload-cn-east-2.qiniup.com";
        QINIU_IOVIP_HOST = "http://iovip-cn-east-2.qiniuio.com";
    }
}

/*============================================================================*/
