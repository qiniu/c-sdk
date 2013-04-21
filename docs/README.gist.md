---
title: C/C++ SDK 使用指南 | 七牛云存储
---

# C/C++ SDK 使用指南

本SDK使用符合C89标准的C语言实现。由于C语言的普适性，原则上此SDK可以跨所有主流平台，不仅可以直接在C和C++的工程中使用，也可以用于与C语言交互性较好的语言中，比如C#（使用P/Invoke交互）、Java（使用JNI交互）、Lua等。本开发指南假设开发者使用的开发语言是C/C++。

SDK下载地址：<https://github.com/qiniu/c-sdk/tags>

**文档大纲**

- [概述](#overview)
- [准备开发环境](#prepare)
    - [环境依赖](#dependences)
    - [ACCESS_KEY 和 SECRET_KEY](#appkey)
- [初始化环境与清理](#init)
- [C-SDK惯例](#convention)
    - [HTTP客户端](#http-client)
    - [错误处理与调试](#error-handling)
- [上传文件](#io-put)
- [下载文件](#io-get)
    - [下载私有文件](io-get-private)
    - [断点续下载](#resumable-io-get)
- [资源操作](#rs)
    - [获取文件信息](#rs-stat)
    - [删除文件](#rs-delete)
    - [复制/移动文件](#rs-copy-move)
    - [批量操作](#rs-batch)
- [断点续上传](#resumable-io-put)

<a name="overview"></a>

## 概述

七牛云存储的C语言版本SDK（本文以下称C-SDK）是对七牛云存储API协议的一层封装，以提供一套对于C/C++开发者而言简单易用的原生C语言函数。C/C++开发者在对接C-SDK时无需理解七牛云存储API协议的细节，原则上也不需要对HTTP协议和原理做非常深入的了解，但如果拥有基础的HTTP知识，对于出错场景的处理可以更加高效。

C-SDK以开源方式提供。开发者可以随时从本文档提供的下载地址查看和下载SDK的源代码，并按自己的工程现状进行合理使用，比如编译为静态库或者动态库后进行链接，或者直接将SDK的源代码加入到自己的工程中一起编译，以保持工程设置的简单性。

由于C语言的通用性，C-SDK被设计为同时适合服务器端和客户端使用。服务端是指开发者自己的业务服务器，客户端是指开发者提供给终端用户的软件，通常运行在iPhone/iPad/Android移动设备，或者运行在Windows/Mac/Linux这样的桌面上。服务端因为有七牛颁发的 AccessKey/SecretKey，可以做很多客户端做不了的事情，比如删除文件、移动/复制文件等操作。除非开发者将文件设置为公开，客服端操作私有文件需要获得服务端的授权。客户端上传文件需要获得服务端颁发的uptoken（上传授权凭证），客户端下载文件（包括下载处理过的文件，比如下载图片的缩略图）需要获得服务端颁发的dntoken（下载授权凭证）。

注意从v5.0.0版本开始，我们对SDK的内容进行了精简。所有管理操作，比如：创建Bucket、删除Bucket、为Bucket绑定域名（publish）、设置数据处理的样式分隔符（fop seperator）和样式（fop style）等都去除了，统一建议到[开发者后台](https://dev.qiniutek.com/)来完成。另外，此前服务端还有自己独有的上传API，现在也推荐统一成基于客户端上传的工作方式。

从内容上来说，C-SDK 主要包含如下几方面的内容：

* 公共部分，所有用况下都用到：qiniu/base.c, qiniu/conf.c, qiniu/oauth2.c
* 客户端上传文件：qiniu/base_io.c, qiniu/io.c
* 客户端断点续上传：qiniu/base_io.c, qiniu/resumable_io.c
* 数据处理：qiniu/fop.c
* 服务端操作：qiniu/oauth2_digest.c (授权), qiniu/rs.c (资源操作), qiniu/rs_token.c (uptoken/dntoken颁发)


<a name="prepare"></a>

## 准备开发环境

<a name="dependences"></a>

### 环境依赖

C-SDK 使用[cURL](http://curl.haxx.se/)进行网络相关操作。无论是作为客户端还是服务端，都需要依赖[cURL](http://curl.haxx.se/)。如果作为服务端，C-SDK 因为需要用HMAC进行数字签名做授权（简称签名授权），所以依赖了[OpenSSL](http://www.openssl.org/)库。C-SDK 并没有带上这两个外部库，因此在使用C-SDK之前需要先确认您的当前开发环境中是否已经安装了这所需的外部库，并且已经将它们的头文件目录和库文件目录都加入到了项目工程的设置。

在主流的*nix环境下，通常[cURL](http://curl.haxx.se/)和[OpenSSL](http://www.openssl.org/)都已经随系统默认安装到`/usr/include`和`/usr/lib`目录下。如果你的系统还没有这些库，请自行安装。如何安装这些第三方库不在本文讨论范围，请自行查阅相关文档。

如果你使用gcc进行编译，服务端典型的链接选项是：`-lcurl -lssl -lcrypto -lm`，客户端则是：`-lcurl -lm`。

如果在项目构建过程中出现环境相关的编译错误和链接错误，请确认这些选项是否都已经正确配置，以及所依赖的库是否都已经正确的安装。


<a name="appkey"></a>

### ACCESS_KEY 和 SECRET_KEY

如果你的服务端采用C-SDK，那么使用C-SDK前，您需要拥有一对有效的 AccessKey 和 SecretKey 用来进行签名授权。可以通过如下步骤获得：

1. [开通七牛开发者帐号](https://dev.qiniutek.com/signup)
2. [登录七牛开发者自助平台，查看 AccessKey 和 SecretKey](https://dev.qiniutek.com/account/keys) 。

C-SDK 的 conf.h 文件中声明了对应的两个变量：`QINIU_ACCESS_KEY`和`QINIU_SECRET_KEY`。你需要在启动程序之初初始化这两个变量为七牛颁发的 AccessKey 和 SecretKey。


<a name="init"></a>

## 初始化环境与清理

对于服务端而言，常规程序流程是：

    @gist(gist/server.c#init)

对于客户端而言，常规程序流程是：

    @gist(gist/client.c#init)

两者主要的区别在于：

1. 客户端没有 `QINIU_ACCESS_KEY`, `QINIU_SECRET_KEY` 变量（不需要初始化）。
2. 客户端没有签名授权，所以初始化 `Qiniu_Client` 对象应该用 `Qiniu_Client_InitNoAuth` 而不是 `Qiniu_Client_Init`。


<a name="convention"></a>

## C-SDK惯例

C语言是一个非常底层的语言，相比其他高级语言来说，它的代码通常看起来会更啰嗦。为了尽量让大家理解我们的C-SDK，这里需要解释下我们在SDK中的一些惯例做法。

<a name="http-client"></a>

### HTTP客户端

在C-SDK中，HTTP客户端叫`Qiniu_Client`。在某些语言环境中，这个类是线程安全的，多个线程可以共享同一份实例，但在C-SDK中它被设计为线程不安全的。一个重要的原因是我们试图简化内存管理的负担。HTTP请求结果的生命周期被设计成由`Qiniu_Client`负责，在下一次请求时会自动释放上一次HTTP请求的结果。这有点粗暴，但在多数场合是合理的。如果有个HTTP请求结果的数据需要长期使用，你应该复制一份。例如：

    @gist(gist/server.c#stat)

这个例子中，`Qiniu_RS_Stat`请求返回了`Qiniu_Error`和`Qiniu_RS_StatRet`两个结构。其中的 `Qiniu_Error` 类型是这样的：

    @gist(../qiniu/base.c#error)

`Qiniu_RS_StatRet` 类型是这样的：

    @gitst(../qiniu/rs.c#statret)

值得注意的是，`Qiniu_Error.message`、`Qiniu_RS_StatRet.hash`、`Qiniu_RS_StatRet.mimeType` 都声明为 `const char*` 类型，是个只读字符串，并不管理字符串内容的生命周期。这些字符串什么时候失效？下次 `Qiniu_Client` 发生网络API请求时失效。如果你需要长久使用，应该复制一份，比如：

    hash = strdup(ret.hash);


<a name="error-handling"></a>

### 错误处理与调试

在HTTP请求出错的时候，C-SDK统一返回了一个`Qiniu_Error`结构体：

    @gist(../qiniu/base.c#error)

即一个错误码和对应的读者友好的消息。这个错误码有可能是cURL的错误码，表示请求发送环节发生了意外，或者是一个HTTP错误码，表示请求发送正常，服务器端处理请求后返回了HTTP错误码。

如果一切正常，`code`应该是200，即HTTP的OK状态码。如果不是200，则需要对`code`的值进行相应分析。对于低于200的值，可以查看[cURL错误码](http://curl.haxx.se/libcurl/c/libcurl-errors.html)，否则应查看[七牛云存储错误码](http://docs.qiniutek.com/v2/api/code/)。

如果`message`指示的信息还不够友好，也可以尝试把整个HTTP返回包打印出来看看：

    @gist(gist/server.c#debug)


<a name="io-put"></a>

## 上传文件


<a name="io-get"></a>

## 下载文件

<a name="io-get-private"></a>

### 下载私有文件

<a name="resumable-io-get"></a>

### 断点续下载


<a name="rs"></a>

## 资源操作

<a name="rs-stat"></a>

### 获取文件信息

<a name="rs-delete"></a>

### 删除文件

<a name="rs-copy-move"></a>

### 复制/移动文件

<a name="rs-batch"></a>

### 批量操作

<a name="resumable-io-put"></a>

## 断点续上传
