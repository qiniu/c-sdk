/*
 ============================================================================
 Name        : conf.c
 Author      : Qiniu.com
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include "conf.h"

const char *version                     ="6.2.6";

/*============================================================================*/

const char *QINIU_ACCESS_KEY = "<Please apply your access key>";
const char *QINIU_SECRET_KEY = "<Dont send your secret key to anyone>";

//公共服务http域名
const char *QINIU_RS_HOST               = "http://rs.qiniu.com";
const char *QINIU_RSF_HOST              = "http://rsf.qiniu.com";
const char *QINIU_API_HOST              = "http://api.qiniu.com";
const char *QINIU_FUSION_HOST           = "http://fusion.qiniuapi.com";
const char *QINIU_UC_HOST               = "http://uc.qbox.me";

//公共服务https域名
//const char* QINIU_RS_HOST				= "https://rs.qiniu.com";
//const char* QINIU_RSF_HOST            = "https://rsf.qiniu.com";
//const char* QINIU_API_HOST			= "https://api.qiniu.com";
//const char* QINIU_UC_HOST				= "https://uc.qbox.me";

//以下为各个机房的上传http和https域名，请根据需要进行设置

//华东机房上传域名
const char *QINIU_UP_HOST               = "http://upload.qiniu.com";
//const char* QINIU_UP_HOST              = "https://upload.qbox.me";

//华北机房上传域名
//const char *QINIU_UP_HOST               = "http://upload-z1.qiniu.com";
//const char *QINIU_UP_HOST               = "https://upload-z1.qbox.me";

//华南机房上传域名
//const char *QINIU_UP_HOST               = "http://upload-z2.qiniu.com";
//const char *QINIU_UP_HOST               = "https://upload-z2.qbox.me";

//北美机房上传域名
//const char *QINIU_UP_HOST               = "http://upload-na0.qiniu.com";
//const char *QINIU_UP_HOST               = "https://upload-na0.qbox.me";

//以下为各个机房的资源抓取http和https域名，请根据需要进行设置

//华东机房
const char *QINIU_IOVIP_HOST               = "http://iovip.qbox.me";
//const char* QINIU_IOVIP_HOST             = "https://iovip.qbox.me";

//华北机房
//const char *QINIU_IOVIP_HOST             = "http://iovip-z1.qbox.me";
//const char *QINIU_IOVIP_HOST             = "https://iovip-z1.qbox.me";

//华南机房
//const char *QINIU_IOVIP_HOST             = "http://iovip-z2.qbox.me";
//const char *QINIU_IOVIP_HOST             = "https://iovip-z2.qbox.me";

//北美机房
//const char *QINIU_IOVIP_HOST             = "http://iovip-na0.qbox.me";
//const char *QINIU_IOVIP_HOST             = "https://iovip-na0.qbox.me";

/*============================================================================*/

