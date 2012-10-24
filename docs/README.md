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
- [初始化与环境清理](#init)
    - [全局初始化](#global-init)
    - [客户端初始化](#client-init)
- [错误处理](#error-handling)
- [内存管理](#memory-management)
- [认证与授权](#authorization)
    - [认证](#authentication)
    - [获得上传授权](#put-auth)
    - [获得下载授权](#get-auth)
    - [UpToken授权](#put-uptoken)
- [上传文件](#rs-put)
	- [服务器端上传](#put-serverside)
	- [客户端上传](#put-clientside)
- [获取文件信息](#rs-stat)
- [发布资源表](#rs-publish)
- [取消资源表发布](#rs-unpublish)
- [删除文件](#rs-delete)
- [创建资源表](#rs_create)
- [删除资源表](#rs-drop)
- [断点续上传文件](#rs-put-blocks)
    - [术语](#rs-put-blocks-term)
    - [工作模型](#rs-put-blocks-model)
    - [数据结构](#rs-put-blocks-data)
    - [API清单](#rs-put-blocks-api)
    - [示例代码](#rs-put-blocks-sample)
- [图像处理](#img_processing)
    - [查看图片属性信息](#img_info)
    - [查看图片EXIF信息](#img_exif)
    - [获取指定规格的缩略图预览地址](#img_view)
    - [高级图像处理（缩略、裁剪、旋转、转化）](#img_mogr)
    - [高级图像处理（缩略、裁剪、旋转、转化）并持久化](#img_saveas)

<a name="overview"></a>

## 概述

七牛云存储的C语言版本SDK（本文以下称C-SDK）总体上实现的功能与其他语言版本SDK一致，对七牛云存储服务的标准RESTful接口进行了封装，以提供一套对于C/C++开发者而言简单易用的原生C语言函数。C/C++开发者在对接七牛云存储接口时无需理解RESTful接口的细节，原则上也不需要对HTTP协议和原理做非常深入的了解，但如果拥有基础的HTTP知识，对于出错场景的处理可以更加高效。

七牛云存储的独特架构允许个人客户端直接与七牛云存储通讯来读写文件，从而大幅降低企业客户方的服务器压力。因此C-SDK也需要设计为同时适合服务器端和客户端使用。客户端通常为单用户单线程模式，使用起来非常简单。但是C-SDK在服务器端的使用需要支持多线程场景，即多个线程使用同一个企业账号访问七牛云存储服务。C-SDK在设计上也考虑到了这一点，允许多个线程共享同一认证凭证，从而在性能和易用性之间达到一个可接受的平衡点。

C—SDK以开源方式提供。无论是否已经获得开发者资格，所有用户都可以随时从本文档提供的下载地址查看和下载SDK的源代码，并按自己的工程现状进行合理使用，比如编译为静态库或者动态库后进行链接，或者直接将SDK的源代码加入到自己的工程中一起编译，以保持工程设置的简单性。

<a name="prepare"></a>

## 准备开发环境

<a name="dependences"></a>

### 环境依赖

C-SDK不对客户的开发环境和运行环境做过多假设。由于C-SDK依赖于cURL和OpenSSL（需支持哈希运算消息认证码，即HMAC），因此在使用C-SDK之前需要先确认您的当前开发环境中是否已经安装了这两个库，并且已经将它们的头文件目录和库文件目录都加入到了项目工程的设置。

在主流的*nix环境下，通常cURL和OpenSSL已经随系统默认安装到`/usr/include`和`/usr/lib`目录下。头文件分别在`/usr/include/curl`和`/usr/include/openssl`目录下，对应的库文件直接位于`/usr/lib`下，分别命名为libcurl和libssl。开发者在初始化工程时需要记得将两个include目录都加入到头文件查找路径中，并将`/usr/lib`加入到库文件的查找路径中，此外还需要在库的链接列表中加入相应的库名称。因为内置的cJSON依赖于数学库，因此也需要链接libm。

在配置完成后，最终的Makefile应该已包含这些选项：`-I/usr/include/curl -I/usr/include/openssl -L/usr/lib/ -lcurl -lssl -lm`。

如果在项目构建过程中出现环境相关的编译错误和链接错误，请确认这些选项是否都已经正确配置，以及所依赖的库是否都已经正确的安装。

<a name="appkey"></a>

### ACCESS_KEY 和 SECRET_KEY

要接入七牛云存储，您需要拥有一对有效的 Access Key 和 Secret Key 用来进行签名认证。可以通过如下步骤获得：

1. [开通七牛开发者帐号](https://dev.qiniutek.com/signup)
2. [登录七牛开发者自助平台，查看 Access Key 和 Secret Key](https://dev.qiniutek.com/account/keys) 。

C-SDK 的 conf.h 文件中声明了对应的两个变量：`QBOX_ACCESS_KEY`和`QBOX_SECRET_KEY`，用于存储上面提到的AccessKey 和 SecretKey。一般情况下，开发者也只需要处理 conf.h 中这两个变量，其他变量不需要修改其默认值。

<a name="init"></a>

## 初始化与环境清理

在调用各个函数之前，我们需要先初始化调用环境。C-SDK提供了几个初始化的函数供在不同的场合下调用，并提供了匹配的清理函数。在结束使用本SDK后，需要调用相应的清理函数，以避免发生资源泄漏的问题。

下面我们给出一个常规的程序流程：

    /* Key值存储 */
    QBOX_ACCESS_KEY = "<Please apply your access key>";
    QBOX_SECRET_KEY = "<Dont send your secret key to anyone>";

    /* 全局初始化函数，整个进程只需要调用一次 */
    QBox_Global_Init(-1);

    /* 客户端初始化，每个线程只需要调用一次 */
    QBox_Client_Init(&client, bufSize); // thread

    /* 初始化完成，现在可以开始调用业务函数了，比如PutAuth, Get, PutFile等 */

    /* 调用业务函数 */
    QBox_RS_PutAuth(&client, &putAuthRet);

    QBox_RS_Get(&client, &getRet, tableName, key, NULL);

    /* 其他业务函数 … */

    /* 调用结束，现在开始释放资源 */

    /* 每个线程结束时需要调用一次QBox_Client_Cleanup以释放相应资源 */
    QBox_Client_Cleanup(&client);

    /* 全局清理函数，只需要在进程退出时调用一次 */
    QBox_Global_Cleanup();


<a name="global-init"></a>

### 全局初始化

全局初始化函数原型如下：

    /* 所属头文件：oauth2.h */

    void QBox_Global_Init(long flags);

该函数的flags参数与cURL的`curl_global_init`的参数规格一致，参见<http://curl.haxx.se/libcurl/c/curl_global_init.html>以了解参数的可选值。一般情况下，只需使用CURL_GLOBAL_ALL作为传入参数。

<a name="client-init"></a>

### 客户端初始化

为了保证多线程场景下使用的安全性，C-SDK提供的函数进行了针对性的设计。C-SDK使用名为`QBox_Client`的结构体来管理单个客户端的数据。这个结构体包含了一些线程敏感的数据，比如在单次HTTP POST过程中的缓冲区，不能被多个POST过程重用。如果直接在多个线程中共用同一结构体对象，很可能会发生POST数据错乱的情况。因此，在多线程场景下，开发者需要确认不发生共用的情况，或为每个线程构建一个`QBox_Client`结构体，并只在这个线程中使用。

我们在[认证与授权](#authorization)这一节会对如何初始化`QBox_Client`对象做详细介绍。

对于简单的使用场景，`QBox_Client`对象可以作为一个线程函数的临时变量初始化，而对于一些比较复杂的场景，则可以考虑将这个对象指针放置到TLS（线程局部存储）中。

<a name="error-handling"></a>

## 错误处理

C-SDK引入了一个统一的错误处理机制。所有业务函数都会返回一个`QBox_Error`类型的返回值。该类型的定义如下：

    typedef struct _QBox_Error {
      int code;
      const char* message;
    } QBox_Error;

可以得到一个错误码和对应的读者友好的消息。这个错误码有可能是cURL的错误码，表示请求发送环节发生了意外，或者是一个HTTP错误码，表示请求发送正常，服务器端处理请求后返回的HTTP错误码。

如果一切正常，`code`应该是200，即HTTP的OK状态码。如果不是200，则需要对`code`的值进行相应分析。对于低于200的值，可以查看[cURL错误码](http://curl.haxx.se/libcurl/c/libcurl-errors.html)，否则应查看[七牛云存储错误码](http://docs.qiniutek.com/v2/api/code/)。

在打印到控制台或者输出到日志文件时，除了错误码开发者也可以考虑包含对读者友好的错误消息`message`，以降低理解错误的难度并提高解决问题的效率。

<a name="memory-management"></a>

## 内存管理

开发者在使用C-SDK之前，需要非常详细的了解C-SDK的内存管理模型，以避免代码中出现很难理解和解决的问题。

一个非常重要的原则是，一次C-SDK的业务函数调用所返回的数据，在下一次业务函数调用之初就会失效。因此如果有一些返回的信息希望能够比较持久的使用的话，应在函数调用返回后立即拷贝数据内容。我们来用具体的例子说明这个原则。

以下是一个有问题的实现：

	QBox_RS_Get(&client, &getRet, "Bucket", "rs_demo.c", NULL);
	QBox_RS_GetIfNotModified(&client, &getRet, "Bucket", "rs_demo.c", NULL, getRet.hash);

问题在于调用`QBox_RS_GetIfNotModified`函数时传入的最后一个参数`getRet.hash`。业务函数所返回的结构体只包含了若干个指针，而这些指针指向的内存区域在每一次新的业务函数调用时将被清空。比如上面的这个调用，函数`QBox_RS_GetIfNotModified`的实现为先清空相应的内存，然后才会去取getRet.hash所指向的内容。很显然，这将导致无法取到正确的内容。

因此，我们需要将代码调整为如下的结构：

	QBox_RS_Get(&client, &getRet, "Bucket", "rs_demo.c", NULL);
	hash = strdup(getRet.hash); /* 对于需要使用的数据getRet.hash，应该马上复制一份 */

	QBox_RS_GetIfNotModified(&client, &getRet, "Bucket", "rs_demo.c", NULL, hash);
	free(hash); /* strdup返回的指针，需要在使用完成后明确释放相应资源 */

只要认真理解了C-SDK这个内存管理的原则，开发者就可以写出又高效又健壮的代码。

<a name="authorization"></a>

## 认证与授权

<a name="authentication"></a>

### 认证

开发者申请并获得AccessKey/SecretKey后，将两个Key值置入对应全局变量，然后调用`QBox_Client_Init`函数初始化Client对象。

函数原型与接口变量如下：

    extern const char* QBOX_ACCESS_KEY;
    extern const char* QBOX_SECRET_KEY;

    /* 所属头文件：oauth2.h */

    void QBox_Client_Init(QBox_Client* self, size_t bufSize);

<a name="put-auth"></a>

### 获得上传授权

上传文件之前需要取得上传授权。但由于服务器端和客户端不同的定位，我们需要使用不同的授权方式和上传流程。

授权成功后，调用者将得到一个在一定时间内有效的URL。之后向这个URL发出一个合乎规范的POST请求即可将文件上传。C-SDK提供了上传文件的函数，帮助开发者构建这个POST请求。我们将在[上传文件](#rs-put)这一节介绍如何使用该临时有效的URL来上传文件。

要取得上传授权，只需调用`QBox_RS_PutAuth`方法。该函数原型如下：

    /* 所属头文件：rs.h */

    /* QBox_RS_PutAuthEx的简化版，其他功能完全一致 */
    QBox_Error QBox_RS_PutAuth(QBox_RS_Client* client,
      	QBox_RS_PutAuthRet* putAuthRet);

	QBox_Error QBox_RS_PutAuthEx(
		QBox_Client* self, QBox_RS_PutAuthRet* ret, const char* callbackUrl, int expiresIn);


调用者需要先定义一个`QBox_RS_PutAuthRet`结构并作为第二个参数传入以获取授权结果。该结构定义如下：

    /* 所属头文件：rs.h */

    typedef struct _QBox_RS_PutAuthRet {
      const char *url;
      QBox_Int64 expiresIn;
    } QBox_RS_PutAuthRet;


该结构的成员比较简单，就是授权的临时URL和这个URL的有效期。之后[上传文件](#rs-put)时需要用到这些值。用户在获取上传授权时可以试图指定一个授权URL的有效期。但需要注意的是，设置成功的超时时间不一定就是传入的值，因为服务器端需要判断超市时间是否在合理的范围之内。如果不是，则会自动调整。具体设置的超时为多少，可以在返回的`QBox_RS_PutAuthRet`结构中读取。

<a name="get-auth"></a>

### 获得下载授权

如同上传文件，下载文件也需要先获取下载授权。授权成功后调用者得到的也是一个在一定时间内有效的URL。要获取下载授权，我们可以调用`QBox_RS_Get`或`QBox_RS_GetIfNotModified`函数。它们的原型如下：

    /* 所属头文件：rs.h */

    QBox_Error QBox_RS_Get(
      QBox_Client* self, QBox_RS_GetRet* ret,
      const char* tableName, const char* key, const char* attName);

    QBox_Error QBox_RS_GetIfNotModified(
      QBox_Client* self, QBox_RS_GetRet* ret,
      const char* tableName, const char* key, const char* attName, const char* base);


要理解函数中的几个参数`tableName`、`key`、`attName`，我们可以查看[七牛云存储相关术语的基本概念](/v2/api/words/)。

这两个函数返回的都是一个类型为`QBox_RS_GetRet`的结构体。该结构体的定义如下：

    typedef struct _QBox_RS_GetRet {
      const char *url; /* 授权的下载URL */
      const char* hash; /* 文件HASH值 */
      const char* mimeType; /* 文件类型 */
      QBox_Int64 fsize; /* 文件尺寸 */
      QBox_Int64 expiresIn;
    } QBox_RS_GetRet;

函数`QBox_RS_GetIfNotModified`与`QBox_RS_Get`的不同之处在于多了一个`base`参数。这个参数是一个SHA-1值，用于判断所指向的文件是否内容被改动。只有在内容没有被变动时才会返回该文件的下载URL。

该`QBox_RS_GetIfNotModified`函数对于需要判断文件内容是否被修改的场景非常有价值。对于那样的场景，如果没有该函数，则会需要先下载整个文件，然后在客户端计算SHA-1值，仅仅是为了判断是否被更改，显然非常不划算。

<a name="put-uptoken"></a>

### UpToken授权

UpToken授权的主要用途是由业务服务器对上传端进行授权，以便其独立与上传服务器连接、完成本SDK库提供的各种文件操作。本功能由如下两个函数支持：  


    /* 所属头文件：auth_policy.h */

    typedef struct _QBox_AuthPolicy {
        const char* scope;        /* 授权操作域（Bucket名称） */
        const char* callbackUrl;  /* 业务服务器回调通知接口   */
        const char* returnUrl;    /* 表单流转处理重定向接口   */
        int         expires;      /* 授权时长（秒）
                                   * 以调用QBox_MakeUpToken()后的时点作为起点 */
    } QBox_AuthPolicy;

    /* 根据授权策略，生成一个UpToken */
    char* QBox_MakeUpToken(
        const QBox_AuthPolicy* auth
    );


    /* 所属头文件：oauth2.h */

    /* 使用UpToken进行授权 */
    void QBox_Client_InitByUpToken(
        QBox_Client* self,
        const char*  uptoken,
        size_t       bufSize
    );


<a name="rs-put"></a>

## 上传文件

客户端上传与服务器端上传的一个最大的不同，在于客户端的上传不允许覆盖文件（即不能上传2个文件到同一个key中以达到覆盖目的），而服务器端允许此操作。这是出于安全性考虑。

<a name="put-serverside"></a>
#### 服务器端上传

服务器端上传文件相应API的规格与其他API基本一致。函数原型如下：

    /* 所属头文件：rs.h */

    QBox_Error QBox_RS_Put(
    	QBox_Client* self, QBox_RS_PutRet* ret,
    	const char* tableName, const char* key,
    	const char* mimeType, FILE* source,
    	QBox_Int64 fsize, const char* customMeta);

    QBox_Error QBox_RS_PutFile(
    	QBox_Client* self, QBox_RS_PutRet* ret,
    	const char* tableName, const char* key,
	    const char* mimeType, const char* srcFile, const char* customMeta);

这两个API的差别仅在于一个传入的是源文件的路径，另一个是打开的`FILE*`指针。

<a name="put-clientside"></a>
#### 客户端上传

因为服务器端已经在之前进行过认证并持有相应的安全凭证，因此可以直接进行上传。而由于客户端并不持有对RS的安全凭证，从客户端上传文件的流程要比服务器端上传复杂一些。

客户端上传前，需要先请求一个有上传权限的URL。这个过程通过调用`QBox_RS_PutAuth`来完成。关于`QBox_RS_PutAuth`的使用，请参见[获得上传授权](#put-auth)。

在获得上传授权URL后，现在我们可以开始上传文件了。上传文件的函数原型如下：

    /* 所属头文件：rscli.h */

    QBox_Error QBox_RSCli_PutFile(
      QBox_Buffer* resp, const char* url,
      const char* tableName, const char* key,
      const char* mimeType, const char* localFile,
      const char* customMeta, const char* callbackParams);

调用者可以通过设置`mimeType`来强行指定文件的类型，比如“text/plain”，也可以留为0，由服务器端按文件内容进行智能判断。

如果调用者希望能够获取详细的相应内容，可以传入一个`QBox_Buffer*`结构体。该结构体可以通过调用`base.h`中的一系列相关函数来初始化、使用和清理。

关于最后两个参数`customMeta`和`callbackParams`，请查看[七牛云存储相关术语的基本概念](/v2/api/words/)。

<a name="rs-stat"></a>

## 获取文件信息

若不想获取下载授权而是指期望获取七牛云存储中某个文件的属性信息，开发者可以调用`QBox_RS_Stat`函数。该函数的原型如下：

    /* 所属头文件：rs.h */

    QBox_Error QBox_RS_Stat(
      QBox_Client* self, QBox_RS_StatRet* ret, const char* tableName, const char* key);


该函数将返回一个类型为`QBox_RS_StatRet`的结构体。该结构体的定义如下：

    typedef struct _QBox_RS_StatRet {
      const char* hash;
      const char* mimeType;
      QBox_Int64 fsize;
      QBox_Int64 putTime; /* 该文件的上传时间 */
    } QBox_RS_StatRet;

<a name="rs-publish"></a>

## 发布资源表

开发者可以发布七牛云存储上的一个资源表。一个被发布的资源表下所有文件都可以被直接匿名访问。该功能可以很好的满足静态网站托管和图床的需求。在发布资源表时可以通过指定一个域名，之后即可通过该域名后接文件的key直接访问对应的文件内容。比如域名指定为`cdn.example.com`的话，即可通过类似于`cdn.example.com/pic2012-5-16-001`这样的域名来直接访问名为`pic2012-5-16-001`的文件内容。

开发者可通过调用`QBox_RS_Publish`函数来发布资源。该函数原型如下：

    /* 所属头文件：rs.h */

    QBox_Error QBox_RS_Publish(QBox_Client* self, const char* tableName, const char* domain);


需要注意的是，开发者需要在域名DNS提供商那里对所指定的域名做一些设置才能让这个机制运作起来。同样使用以上的例子，如果 example.com 使用的是在 Godaddy.com 的DNS服务，则域名所有人需要登录 Godaddy.com 域名管理控制面板，增加一个 cdn.example.com 子域名，并针对此子域名增加一个 CNAME 记录，并将值设为`iovip.qbox.me`。

<a name="rs-unpublish"></a>

## 取消发布资源表

可以通过调用`QBox_RS_Unpublish`函数取消发布某个资源表，从而将该资源表恢复为不可匿名访问状态。该函数的原型如下：

    /* 所属头文件：rs.h */

    QBox_Error QBox_RS_Unpublish(QBox_Client* self, const char* domain);


<a name="rs-delete"></a>

## 删除文件

要删除指定的文件，只需调用`QBox_RS_Delete`函数。函数原型如下：


    /* 所属头文件：rs.h */

    QBox_Error QBox_RS_Delete(QBox_Client* self, const char* tableName, const char* key);


<a name="rs-create"></a>
## 创建资源表

调用`QBox_RS_Create`函数可以删除整个资源表。该函数原型如下：

	/* 所属头文件：rs.h */

	QBox_Error QBox_RS_Create(QBox_Client* self, const char* tableName);


<a name="rs-drop"></a>

## 删除资源表

调用`QBox_RS_Drop`函数可以删除整个资源表。该函数原型如下：

    /* 所属头文件：rs.h */

    QBox_Error QBox_RS_Drop(QBox_Client* self, const char* tableName);


删除资源表是一个非常危险的动作，调用应非常慎重，因为此操作将导致该表包含的所有文件都不可访问。

<a name="rs-put-blocks"></a>

## 断点续上传文件

七牛云存储现已提供断点续上传功能。该功能将单个文件分割成数个固定大小的块分开上传，可以在实现断点续传的同时加快上传速度（并发上传）。

<a name="rs-put-blocks-term"></a>

### 术语

1. 上传服务器（Up-Server）：提供断点续上传功能的服务器，负责启动新的上传过程、接受上传内容、合并生成最终上传文件。

2. 业务服务器（Biz-Server）：七牛云存储的客户的业务服务器，负责上传操作鉴权、分配操作策略、生成UpToken、驱动上传端启动上传。

3. 上传凭证（UpToken）：由业务服务器使用AccessKey和SecretKey，对操作策略进行数字签名防伪而生成的上传凭证。

4. 操作策略（Policy）：由业务服务器填写、由上传服务器执行的操作信息。  
    4.1. 操作域（Scope）：1）空，表示可以上传到任意Bucket（仅限于新增文件）；2) "Bucket"，表示限定只能传到该Bucket（仅限于新增文件）；3) "Bucket:Key"，表示限定特定的文档，可新增或修改文件;
    4.2. 超时时限（DeadLine）：指定自调用`QBox_Client_MakeUpToken`函数开始，本次操作策略的有效时间，单位是秒;  
    4.3. 回调URL（CallbackURL）：如果指定，在合并文件后，由上传服务器调用此URL，以通知业务服务器做相应处理;  
    4.4. 返回URL（ReturnURL）：如果指定，在合并文件后，由上传服务器重定向到此URL，以通知客户端继续表单处理流程。

5. 上传端（Up-Client）：七牛云存储的客户的业务终端，负责提出、实施上传。

6. 分割块（Block）：分割块是以4MB为单位分割待传文件得到的内容块。最后一个分割块可以小于4MB。不同分割块可以乱序并行上传。所有分割块上传完毕后由服务器进行内容排序合并。

7. 上传块（Chunk）：上传块是对分割块的进一步切分，可以由用户自行设定大小，以适应不同网络环境的限制。上传块必须顺序上传，同时根据需求保存上传过程中的上下文信息、同一分割块已传部分的校验值，以便在断点续传时恢复操作环境。

8. 上下文信息（Context）：服务器成功保存上传块后返回的操作环境信息，可保存在上传端本地，以便恢复操作环境。上传开始后，每个分割块都有自己的上下文信息。上传端不能修改接收到的上下文信息。

9. 校验值（CheckSum）：服务器成功保存上传块后返回的、当前分割块的已传部分的校验值，可保存在上传端本地，用于最后合并文件。上传开始后，每个分割块都有自己的校验值。上传端不能修改接收到的校验值。

<a name="rs-put-blocks-model"></a>

### 工作模型

                                                        |
      CUSTOM                                            | QINIU
                                                        |
            +------------+                              |     +------------+
            |            |    6.1 Invoke CallbackURL    |     |            |
            |            | <----------------------------+---- |            |
            | Biz-Server |                              |     | Up-Server  |
            |            | -----------------------------+---> |            |
            |            |    6.2 Return Result         |     |            |
            +------------+                              |     +------------+
                 ^  |                                   |       ^ |    ^ |
                 |  |                                   |       | |    | |
                 |  |                                   |       | |    | |
      1 Request  |  | 2 Make Policy/                    |       | |    | |
        Upload   |  |   UpToken                         |       | |    | |
                 |  |                                   |       | |    | |
      -----------+--+-----------                        |       | |    | |
                 |  |                                   |       | |    | |
      END-USER   |  |                                   |       | |    | |
                 |  V                                   |       | |    | |
            +------------+    4.1 Upload Blocks         |       | |    | |
            |            | -----------------------------+-------/ |    | |
            |            | <----------------------------+---------/    | |
            |            |    4.2 Return Context/       |              | |
            |            |        CheckSum              |              | |
            | Up-Client  |                              |              | |
            |            |                              |              | |
            |            |    5   Make File             |              | |
      3 Split File       | -----------------------------+--------------/ |
            |            | <----------------------------+----------------/
            +------------+    6.3 Return Result         |
                                                        |
                                                        |

1. 请求断点续上传（Request Upload）：由上传端发起，向业务服务器申请执行断点续上传;  

2. 生成操作策略/上传凭证（Make Policy/UpToken）：业务服务器对上传端进行鉴权/签名上传凭证/授权;  

3. 分割文件（Split File）：上传端获得授权后，以4MB为单位，将待传文件分割为数个分割块;  

4. 上传分割块（Upload Blocks）：上传端将单个分割块至上传服务器（可以并发上传不同的分割块，加快上传速度）。每个分割块的上传过程必须顺序完成（串行上传每个上传块）。上传服务器会针对接受到的上传块，返回对应分割块的已上传部分的上下文信息和校验码;  

5. 合并文件（Make File）：所有分割块均成功上传完毕后，由上传端通知上传服务器将其合并成原上传对象文件;  

6. 若指定CallbackURL，上传服务器在合并文件后会调用此URL，通知业务服务器做相应处理; 否则返回响应结果。  

注：在第4步的任何节点均可终止（Abort）上传分割块，或在断点处根据上下文信息恢复上传分割块。

<a name="rs-put-blocks-data"></a>

### 数据结构

    /* 所属头文件：up.h */

    /* 分割块上传进度对象 */
    typedef struct _QBox_UP_BlockProgress {
        char* ctx;    /* 已传部分的上下文信息（Base64编码）         */
        int offset;   /* 已传部分的偏移（相对于分割块绝对偏移计算） */
        int restSize; /* 未传部分长度                               */
        int errCode;  /* HTTP状态码                                 */
    } QBox_UP_BlockProgress;

    /* 上传结果对象 */
    typedef struct _QBox_UP_PutRet {
        const char* ctx;       /* 已传部分的上下文信息      */
        const char* checksum;  /* 已传部分的校验码          */
        QBox_Uint32 crc32;     /* 本次上传部分的CRC32校验码 */
    } QBox_UP_PutRet;

    /* 校码码对象 */
    typedef struct _QBox_UP_Checksum {
        char value[28];        /* URL-Safe-Base64-Encoded */
    } QBox_UP_Checksum;

    /* 上传进度对象
     * 使用断点续传功能时可根据需要持久化本对象，
     * 用于恢复上传环境 */
    typedef struct _QBox_UP_Progress {
        int blockCount;                /* 上传文件的分割块数目
                                        * （文件大小 / 分割块大小） */

        QBox_UP_Checksum* checksums;   /* 校验码对象集合，
                                        * 含blockCount个QBox_UP_Checksum对象 */

        QBox_UP_BlockProgress* progs;  /* 分割块上传进度对象集合，
                                        * 含blockCount个QBox_UP_BlockProgress对象 */
    } QBox_UP_Progress;

<a name="rs-put-blocks-api"></a>

### API清单

    /* 所属头文件：base.h */

    /* 上传块数据读取接口 */
    typedef	ssize_t (*QBox_FnReadAt)(
        void*         self,
        void*         buf,
        size_t        bytes,
        off_t         offset
    );

    typedef struct _QBox_ReaderAt {
        void*         self;
        QBox_FnReadAt ReadAt;
    } QBox_ReaderAt;


    /* 所属头文件：up.h */

    /* 上传块回调接口：成功上传一个上传块后调用（可选） */
    typedef void (*QBox_UP_FnChunkNotify)(
        void*                  self,
        int                    blockIndex,
        QBox_UP_BlockProgress* prog
    );

    /* 分割块回调接口：成功完整上传一个分割块后调用（可选） */
    typedef void (*QBox_UP_FnBlockNotify)(
        void*                  self,
        int                    blockIndex,
        QBox_UP_Checksum*      checksum
    );

    /* 为指定大小的上传文件生成进度对象 */
    QBox_UP_Progress* QBox_UP_NewProgress(
        QBox_Int64             fileSize
    );

    /* 释放进度对象 */
    void QBox_UP_Progress_Release(
        QBox_UP_Progress*      prog
    );

    /* 向上传服务器发起请求，
     * 同时上传第一个上传块 */
    QBox_Error QBox_UP_Mkblock(
        QBox_Client*           self,
        QBox_UP_PutRet*        ret,
        int                    blockSize,
        QBox_Reader            body,
        int                    bodyLength
    );

    /* 上传单个上传块
     * 必须在QBox_UP_Mkblock()后调用 */
    QBox_Error QBox_UP_Blockput(
        QBox_Client*           self,
        QBox_UP_PutRet*        ret,
        const char*            ctx,
        int                    offset,
        QBox_Reader            body,
        int                    bodyLength
    );

    /* 以可断点续传的形式上传一个完整的分割块 */
    QBox_Error QBox_UP_ResumableBlockput(
        QBox_Client*           self,
        QBox_UP_PutRet*        ret,
        QBox_ReaderAt          f,
        int                    blockIndex,
        int                    blockSize,
        int                    chunkSize,
        int                    retryTimes,
        QBox_UP_BlockProgress* prog,
        QBox_UP_FnChunkNotify  chunkNotify,
        void*                  notifyParams
    );

    /* 合并所有已上传的分割块，还原成上传文件
     * 并存入指定Bucket */
    QBox_Error QBox_UP_Mkfile(
        QBox_Client*           self,
        QBox_Json**            ret,
        const char*            cmd,
        const char*            entry,
        const char*            mimeType,
        QBox_Int64             fileSize,
        const char*            params,
        const char*            callbackParams,
        QBox_UP_Checksum*      checksums,
        int                    blockCount
    );

    /* 完整实现断点续上传操作
     * 是对上述函数的包装层 */
    QBox_Error QBox_UP_Put(
        QBox_Client*           self,
        QBox_UP_PutRet*        ret,
        QBox_ReaderAt          f,
        QBox_Int64             fileSize,
        QBox_UP_Progress*      prog,
        QBox_UP_FnBlockNotify  blockNotify,
        QBox_UP_FnChunkNotify  chunkNotify,
        void*                  notifyParams
    );


    /* 所属头文件：rs.h */

    /* 以断点上传形式上传一个文件 */
    QBox_Error QBox_RS_ResumablePut(
        QBox_Client*           self,
        QBox_UP_PutRet*        ret,            /* 调用结束时，
                                                * 最后一个上传块的操作结果 */
        QBox_UP_Progress*      prog,
        QBox_UP_FnBlockNotify  blockNotify,
        QBox_UP_FnChunkNotify  chunkNotify,
        void*                  notifyParams,   /* 进度回调接口对象 */
        const char*            entryURI,       /* 格式：Bucket:Key */
        const char*            mimeType,
        QBox_ReaderAt          f,              /* 上传内部读取接口 */
        QBox_Int64             fileSize,       /* 文件大小         */
        const char*            customMeta,     /* 文件元信息       */
        const char*            callbackParams  /* 回调接口参数     */
    );

<a name="rs-put-blocks-sample"></a>

### 示例代码（c-sdk/examples/resumable_put/up_demo.c）

    /* 断点续传文件 */
    QBox_Error err;
    QBox_Client client;
    QBox_AuthPolicy auth;
    QBox_ReaderAt f;
    QBox_UP_PutRet putRet;

    /* 必要的初始化 */
    QBox_Zero(client);
    QBox_Zero(auth);

    /* 生成UpToken（应由Biz-Server调用） */
	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
		return;
	}

    /* 对Client授权（应由Up-Client调用） */
    QBox_Client_InitByUpToken(&client, uptoken, 1024);

    /* 生成上传块读取接口对象 */
    f = QBox_FileReaderAt_Open(fl);

    if ((int)f.self >= 0) {
        fsize = (QBox_Int64) lseek((int)f.self, 0, SEEK_END);

        /* 生成上传进度对象 */
        prog = QBox_UP_NewProgress(fsize);

        err = QBox_RS_ResumablePut(
            &client,
            &putRet,
            prog,
            NULL, /* no blockNotify    */
            NULL, /* no chunkNotify    */
            NULL, /* no notifyParams   */
            "Bucket:up_demo.c",
            "text/plain",
            f,
            fsize,
            NULL, /* no customMeta     */
            NULL  /* no callbackParams */
        );
        free(entry);

        QBox_FileReaderAt_Close(f.self);

        if (err.code != 200) {
            free(uptoken);
            return;
        }

        /* 释放上传进度对象 */    
        QBox_UP_Progress_Release(prog);
    }

