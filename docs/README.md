---
title: C/C++ SDK
---

本 SDK 使用符合 C89 标准的 C 语言实现。由于 C 语言的普适性，原则上此 SDK 可以跨所有主流平台，不仅可以直接在 C 和 C++ 的工程中使用，也可以用于与 C 语言交互性较好的语言中，比如 C#（使用P/Invoke交互）、Java（使用JNI交互）、Lua 等。本开发指南假设开发者使用的开发语言是 C/C++。

SDK 下载地址：<https://github.com/qiniu/c-sdk/tags>

**文档大纲**

- [概述](#overview)
- [准备开发环境](#prepare)
    - [环境依赖](#dependences)
    - [ACCESS_KEY 和 SECRET_KEY](#appkey)
- [初始化环境与清理](#init)
- [C-SDK惯例](#convention)
    - [内存管理](#memory-manage)
    - [HTTP客户端](#http-client)
    - [错误处理与调试](#error-handling)
- [上传文件](#io-put)
    - [上传流程](#io-put-flow)
    - [上传策略](#io-put-policy)
    - [断点续上传、分块并行上传](#resumable-io-put)
- [下载文件](#io-get)
    - [下载私有文件](#io-get-private)
    - [HTTPS 支持](#io-https-get)
    - [断点续下载](#resumable-io-get)
- [资源操作](#rs)
    - [获取文件信息](#rs-stat)
    - [删除文件](#rs-delete)
    - [复制/移动文件](#rs-copy-move)
    - [批量操作](#rs-batch)

<a name="overview"></a>

## 概述

七牛云存储的 C 语言版本 SDK（本文以下称 C-SDK）是对七牛云存储API协议的一层封装，以提供一套对于 C/C++ 开发者而言简单易用的原生 C 语言函数。C/C++ 开发者在对接 C-SDK 时无需理解七牛云存储 API 协议的细节，原则上也不需要对 HTTP 协议和原理做非常深入的了解，但如果拥有基础的 HTTP 知识，对于出错场景的处理可以更加高效。

C-SDK 以开源方式提供。开发者可以随时从本文档提供的下载地址查看和下载 SDK 的源代码，并按自己的工程现状进行合理使用，比如编译为静态库或者动态库后进行链接，或者直接将 SDK 的源代码加入到自己的工程中一起编译，以保持工程设置的简单性。

由于 C 语言的通用性，C-SDK 被设计为同时适合服务器端和客户端使用。服务端是指开发者自己的业务服务器，客户端是指开发者提供给终端用户的软件，通常运行在 iPhone/iPad/Android 移动设备，或者运行在 Windows/Mac/Linux 这样的桌面平台上。服务端因为有七牛颁发的 AccessKey/SecretKey，可以做很多客户端做不了的事情，比如删除文件、移动/复制文件等操作。一般而言，客服端操作文件需要获得服务端的授权。客户端上传文件需要获得服务端颁发的 [uptoken（上传授权凭证）](http://docs.qiniu.com/api/put.html#uploadToken)，客户端下载文件（包括下载处理过的文件，比如下载图片的缩略图）需要获得服务端颁发的 [dntoken（下载授权凭证）](http://docs.qiniu.com/api/get.html#download-token)。但开发者也可以将 bucket 设置为公开，此时文件有永久有效的访问地址，不需要业务服务器的授权，这对网站的静态文件（如图片、js、css、html）托管非常方便。

从 v5.0.0 版本开始，我们对 SDK 的内容进行了精简。所有管理操作，比如：创建/删除 bucket、为 bucket 绑定域名（publish）、设置数据处理的样式分隔符（fop seperator）、新增数据处理样式（fop style）等都去除了，统一建议到[开发者后台](https://portal.qiniu.com/)来完成。另外，此前服务端还有自己独有的上传 API，现在也推荐统一成基于客户端上传的工作方式。

从内容上来说，C-SDK 主要包含如下几方面的内容：

* 公共部分，所有用况下都用到：qiniu/base.c, qiniu/conf.c, qiniu/http.c
* 客户端上传文件：qiniu/base_io.c, qiniu/io.c
* 客户端断点续上传：qiniu/base_io.c, qiniu/io.c, qiniu/resumable_io.c
* 数据处理：qiniu/fop.c
* 服务端操作：qiniu/auth_mac.c (授权), qiniu/rs.c (资源操作, uptoken/dntoken颁发)


<a name="prepare"></a>

## 准备开发环境

<a name="dependences"></a>

### 环境依赖

C-SDK 使用 [cURL](http://curl.haxx.se/) 进行网络相关操作。无论是作为客户端还是服务端，都需要依赖 [cURL](http://curl.haxx.se/)。如果作为服务端，C-SDK 因为需要用 HMAC 进行数字签名做授权（简称签名授权），所以依赖了 [OpenSSL](http://www.openssl.org/) 库。C-SDK 并没有带上这两个外部库，因此在使用 C-SDK 之前需要先确认您的当前开发环境中是否已经安装了这所需的外部库，并且已经将它们的头文件目录和库文件目录都加入到了项目工程的设置。

在主流的 *nix 环境下，通常 [cURL](http://curl.haxx.se/) 和 [OpenSSL](http://www.openssl.org/) 都已经随系统默认安装到`/usr/include`和`/usr/lib`目录下。如果你的系统还没有这些库，请自行安装。如何安装这些第三方库不在本文讨论范围，请自行查阅相关文档。

如果你使用 gcc 进行编译，服务端典型的链接选项是：`-lcurl -lssl -lcrypto -lm`，客户端则是：`-lcurl -lm`。

如果在项目构建过程中出现环境相关的编译错误和链接错误，请确认这些选项是否都已经正确配置，以及所依赖的库是否都已经正确的安装。


<a name="appkey"></a>

### ACCESS_KEY 和 SECRET_KEY

如果你的服务端采用 C-SDK，那么使用 C-SDK 前，您需要拥有一对有效的 AccessKey 和 SecretKey 用来进行签名授权。可以通过如下步骤获得：

1. [开通七牛开发者帐号](https://portal.qiniu.com/signup)
2. [登录七牛开发者自助平台，查看 AccessKey 和 SecretKey](https://portal.qiniu.com/setting/key) 。

C-SDK 的 conf.h 文件中声明了对应的两个变量：`QINIU_ACCESS_KEY`和`QINIU_SECRET_KEY`。你需要在启动程序之初初始化这两个变量为七牛颁发的 AccessKey 和 SecretKey。


<a name="init"></a>

## 初始化环境与清理

对于服务端而言，常规程序流程是：

```{c}
Qiniu_Client client;

QINIU_ACCESS_KEY = "<Please apply your access key>";
QINIU_SECRET_KEY = "<Dont send your secret key to anyone>";

Qiniu_Servend_Init(-1);                        /* 全局初始化函数，整个进程只需要调用一次 */
Qiniu_Client_InitMacAuth(&client, 1024, NULL); /* HTTP客户端初始化。HTTP客户端是线程不安全的，不要在多个线程间共用 */

...

Qiniu_Client_Cleanup(&client);                 /* 每个HTTP客户端使用完后释放 */
Qiniu_Servend_Cleanup();                       /* 全局清理函数，只需要在进程退出时调用一次 */
```

对于客户端而言，常规程序流程是：

```{c}
Qiniu_Client client;

Qiniu_Global_Init(-1);                  /* 全局初始化函数，整个进程只需要调用一次 */
Qiniu_Client_InitNoAuth(&client, 1024); /* HTTP客户端初始化。HTTP客户端是线程不安全的，不要在多个线程间共用 */

...

Qiniu_Client_Cleanup(&client);          /* 每个HTTP客户端使用完后释放 */
Qiniu_Global_Cleanup();                 /* 全局清理函数，只需要在进程退出时调用一次 */
```

两者主要的区别在于：

1. 客户端没有 `QINIU_ACCESS_KEY`, `QINIU_SECRET_KEY` 变量（不需要初始化）。
2. 客户端没有签名授权，所以初始化 `Qiniu_Client` 对象应该用 `Qiniu_Client_InitNoAuth` 而不是 `Qiniu_Client_InitMacAuth`。
3. 客户端初始化/清理用 Qiniu_Global_Init/Cleanup，而服务端用 Qiniu_Servend_Init/Cleanup 这对函数。


<a name="convention"></a>

## C-SDK 惯例

C 语言是一个非常底层的语言，相比其他高级语言来说，它的代码通常看起来会更啰嗦。为了尽量让大家理解我们的 C-SDK，这里需要解释下我们在 SDK 中的一些惯例做法。

<a name="memory-manage"></a>

## 内存管理

在 C-SDK 中，有一些函数会涉及到内存的动态分配。这些函数的一惯处理方式为在函数内部申请内存，并以指针的形式直接返回。这就要求函数调用者在得到指针后，需要在恰当的时机去释放这些内存。对于特殊的结构体，C-SDK 都会提供特定的函数来释放内存，比如 Qiniu_Buffer 提供了 Qiniu_Buffer_Cleanup 函数。而对于其他基本数据类型的指针，则由 Qiniu_Free 函数来负责释放不再使用的内存。

<a name="http-client"></a>

### HTTP 客户端

在 C-SDK 中，HTTP 客户端叫`Qiniu_Client`。在某些语言环境中，这个类是线程安全的，多个线程可以共享同一份实例，但在 C-SDK 中它被设计为线程不安全的。一个重要的原因是我们试图简化内存管理的负担。HTTP 请求结果的生命周期被设计成由`Qiniu_Client`负责，在下一次请求时会自动释放上一次 HTTP 请求的结果。这有点粗暴，但在多数场合是合理的。如果某个 HTTP 请求结果的数据需要长期使用，你应该复制一份。例如：

```{c}
void stat(Qiniu_Client* client, const char* bucket, const char* key)
{
	Qiniu_RS_StatRet ret;
	Qiniu_Error err = Qiniu_RS_Stat(client, &ret, bucket, key);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("hash: %s, fsize: %lld, mimeType: %s\n", ret.hash, ret.fsize, ret.mimeType);
}
```

这个例子中，`Qiniu_RS_Stat`请求返回了`Qiniu_Error`和`Qiniu_RS_StatRet`两个结构体。其中的 `Qiniu_Error` 类型是这样的：

```{c}
typedef struct _Qiniu_Error {
	int code;
	const char* message;
} Qiniu_Error;
```

`Qiniu_RS_StatRet` 类型是这样的：

```{c}
typedef struct _Qiniu_RS_StatRet {
	const char* hash;
	const char* mimeType;
	Qiniu_Int64 fsize;	
	Qiniu_Int64 putTime;
} Qiniu_RS_StatRet;
```

值得注意的是，`Qiniu_Error.message`、`Qiniu_RS_StatRet.hash`、`Qiniu_RS_StatRet.mimeType` 都声明为 `const char*` 类型，是个只读字符串，并不管理字符串内容的生命周期。这些字符串什么时候失效？下次 `Qiniu_Client` 发生网络 API 请求时失效。如果你需要长久使用，应该复制一份，比如：

    hash = strdup(ret.hash);


<a name="error-handling"></a>

### 错误处理与调试

在 HTTP 请求出错的时候，C-SDK 统一返回了一个`Qiniu_Error`结构体：

```{c}
typedef struct _Qiniu_Error {
	int code;
	const char* message;
} Qiniu_Error;
```

即一个错误码和对应的读者友好的消息。这个错误码有可能是 cURL 的错误码，表示请求发送环节发生了意外，或者是一个 HTTP 错误码，表示请求发送正常，服务器端处理请求后返回了 HTTP 错误码。

如果一切正常，`code`应该是 200，即 HTTP 的 OK 状态码。如果不是 200，则需要对`code`的值进行相应分析。对于低于 200 的值，可以查看 [cURL 错误码](http://curl.haxx.se/libcurl/c/libcurl-errors.html)，否则应查看[七牛云存储错误码](http://docs.qiniu.com/api/put.html#error-code)。

如果`message`指示的信息还不够友好，也可以尝试把整个 HTTP 返回包打印出来看看：

```{c}
void debug(Qiniu_Client* client, Qiniu_Error err)
{
	printf("\nerror code: %d, message: %s\n", err.code, err.message);
	printf("respose header:\n%s", Qiniu_Buffer_CStr(&client->respHeader));
	printf("respose body:\n%s", Qiniu_Buffer_CStr(&client->b));
}
```

<a name="io-put"></a>

## 上传文件

为了尽可能地改善终端用户的上传体验，七牛云存储首创了客户端直传功能。一般云存储的上传流程是：

    客户端（终端用户） => 业务服务器 => 云存储服务

这样多了一次上传的流程，和本地存储相比，会相对慢一些。但七牛引入了客户端直传，将整个上传过程调整为：

    客户端（终端用户） => 七牛 => 业务服务器

客户端（终端用户）直接上传到七牛的服务器，通过DNS智能解析，七牛会选择到离终端用户最近的ISP服务商节点，速度会比本地存储快很多。文件上传成功以后，七牛的服务器使用回调功能，只需要将非常少的数据（比如Key）传给应用服务器，应用服务器进行保存即可。

<a name="io-put-flow"></a>

### 上传流程

在七牛云存储中，整个上传流程大体分为这样几步：

1. 业务服务器颁发 [uptoken（上传授权凭证）](http://docs.qiniu.com/api/put.html#uploadToken)给客户端（终端用户）
2. 客户端凭借 [uptoken](http://docs.qiniu.com/api/put.html#uploadToken) 上传文件到七牛
3. 在七牛获得完整数据后，发起一个 HTTP 请求回调到业务服务器
4. 业务服务器保存相关信息，并返回一些信息给七牛
5. 七牛原封不动地将这些信息转发给客户端（终端用户）

需要注意的是，回调到业务服务器的过程是可选的，它取决于业务服务器颁发的 [uptoken](http://docs.qiniu.com/api/put.html#uploadToken)。如果没有回调，七牛会返回一些标准的信息（比如文件的 hash）给客户端。如果上传发生在业务服务器，以上流程可以自然简化为：

1. 业务服务器生成 uptoken（不设置回调，自己回调到自己这里没有意义）
2. 凭借 [uptoken](http://docs.qiniu.com/api/put.html#uploadToken) 上传文件到七牛
3. 善后工作，比如保存相关的一些信息

服务端生成 [uptoken](http://docs.qiniu.com/api/put.html#uploadToken) 代码如下：

```{c}
char* uptoken(Qiniu_Client* client, const char* bucket)
{
	Qiniu_RS_PutPolicy putPolicy;
	Qiniu_Zero(putPolicy);
	putPolicy.scope = bucket;
	return Qiniu_RS_PutPolicy_Token(&putPolicy, NULL);
}
```

上传文件到七牛（通常是客户端完成，但也可以发生在服务端）：

```{c}
char* upload(Qiniu_Client* client, char* uptoken, const char* key, const char* localFile)
{
	Qiniu_Error err;
	Qiniu_Io_PutRet putRet;
	err = Qiniu_Io_PutFile(client, &putRet, uptoken, key, localFile, NULL);
	if (err.code != 200) {
		debug(client, err);
		return NULL;
	}
	return strdup(putRet.hash); /* 注意需要后续使用的变量要复制出来 */
}
```

如果不感兴趣返回的 hash 值，还可以更简单：

```{c}
int simple_upload(Qiniu_Client* client, char* uptoken, const char* key, const char* localFile)
{
	Qiniu_Error err;
	err = Qiniu_Io_PutFile(client, NULL, uptoken, key, localFile, NULL);
	return err.code;
}
```

<a name="io-put-policy"></a>

### 上传策略

[uptoken](http://docs.qiniu.com/api/put.html#uploadToken) 实际上是用 AccessKey/SecretKey 进行数字签名的上传策略(`Qiniu_RS_PutPolicy`)，它控制则整个上传流程的行为。让我们快速过一遍你都能够决策啥：

```{c}
typedef struct _Qiniu_RS_PutPolicy {
    const char* scope;            // 必选项。可以是 bucketName 或者 bucketName:key
    const char* callbackUrl;      // 可选
    const char* callbackBody;     // 可选
    const char* returnUrl;        // 可选，更贴切的名字是 redirectUrl。
    const char* returnBody;       // 可选
    const char* endUser;          // 可选
    const char* asyncOps;         // 可选
    Qiniu_Uint32 expires;         // 可选。默认是 3600 秒
} Qiniu_RS_PutPolicy;
```

* `scope` 限定客户端的权限。如果 `scope` 是 bucket，则客户端只能新增文件到指定的 bucket，不能修改文件。如果 `scope` 为 bucket:key，则客户端可以修改指定的文件。**注意： key必须采用utf8编码，如使用非utf8编码访问七牛云存储将反馈错误**
* `callbackUrl` 设定业务服务器的回调地址，这样业务服务器才能感知到上传行为的发生。
* `callbackBody` 设定业务服务器的回调信息。文件上传成功后，七牛向业务服务器的callbackUrl发送的POST请求携带的数据。支持 [魔法变量](http://docs.qiniu.com/api/put.html#MagicVariables) 和 [自定义变量](http://docs.qiniu.com/api/put.html#xVariables)。
* `returnUrl` 设置用于浏览器端文件上传成功后，浏览器执行301跳转的URL，一般为 HTML Form 上传时使用。文件上传成功后浏览器会自动跳转到 `returnUrl?upload_ret=returnBody`。
* `returnBody` 可调整返回给客户端的数据包，支持 [魔法变量](http://docs.qiniu.com/api/put.html#MagicVariables) 和 [自定义变量](http://docs.qiniu.com/api/put.html#xVariables)。`returnBody` 只在没有 `callbackUrl` 时有效（否则直接返回 `callbackUrl` 返回的结果）。不同情形下默认返回的 `returnBody` 并不相同。在一般情况下返回的是文件内容的 `hash`，也就是下载该文件时的 `etag`；但指定 `returnUrl` 时默认的 `returnBody` 会带上更多的信息。
* `asyncOps` 可指定上传完成后，需要自动执行哪些数据处理。这是因为有些数据处理操作（比如音视频转码）比较慢，如果不进行预转可能第一次访问的时候效果不理想，预转可以很大程度改善这一点。

关于上传策略更完整的说明，请参考 [uptoken](http://docs.qiniu.com/api/put.html#uploadToken)。


<a name="resumable-io-put"></a>

### 断点续上传、分块并行上传

除了基本的上传外，七牛还支持你将文件切成若干块（除最后一块外，每个块固定为4M大小），每个块可独立上传，互不干扰；每个分块块内则能够做到断点上续传。

我们先看支持了断点上续传、分块并行上传的基本样例：

```{c}
int resumable_upload(Qiniu_Client* client, char* uptoken, const char* key, const char* localFile)
{
	Qiniu_Error err;
	Qiniu_Rio_PutExtra extra;
	Qiniu_Zero(extra);
	extra.bucket = bucket;
	err = Qiniu_Rio_PutFile(client, NULL, uptoken, key, localFile, &extra);
	return err.code;
}
```

相比普通上传，断点上续传代码没有变复杂。基本上就只是将`Qiniu_Io_PutExtra`改为`Qiniu_Rio_PutExtra`，`Qiniu_Io_PutFile`改为`Qiniu_Rio_PutFile`。

但实际上 `Qiniu_Rio_PutExtra` 多了不少配置项，其中最重要的是两个回调函数：`notify` 与 `notifyErr`，它们用来通知使用者有更多的数据被传输成功，或者有些数据传输失败。在 `notify` 回调函数中，比较常见的做法是将传输的状态进行持久化，以便于在软件退出后下次再进来还可以继续进行断点续上传。但不传入 `notify` 回调函数并不表示不能断点续上传，只要程序没有退出，上传失败自动进行续传和重试操作。


<a name="io-get"></a>

## 下载文件

每个 bucket 都会绑定一个或多个域名（domain）。如果这个 bucket 是公开的，那么该 bucket 中的所有文件可以通过一个公开的下载 url 可以访问到：

    http://<domain>/<key>

其中\<domain\>是bucket所对应的域名。七牛云存储为每一个bucket提供一个默认域名。默认域名可以到[七牛云存储开发者平台](https://portal.qiniu.com/)中，空间设置的域名设置一节查询。

假设某个 bucket 既绑定了七牛的二级域名，如 hello.qiniudn.com，也绑定了自定义域名（需要备案），如 hello.com。那么该 bucket 中 key 为 a/b/c.htm 的文件可以通过 http://hello.qiniudn.com/a/b/c.htm 或 http://hello.com/a/b/c.htm 中任意一个 url 进行访问。**注意： key必须采用utf8编码，如使用非utf8编码访问七牛云存储将反馈错误**

<a name="io-get-private"></a>

### 下载私有文件

如果某个 bucket 是私有的，那么这个 bucket 中的所有文件只能通过一个的临时有效的 downloadUrl 访问：

    http://<domain>/<key>?e=<deadline>&token=<dntoken>

其中 dntoken 是由业务服务器签发的一个[临时下载授权凭证](http://docs.qiniu.com/api/get.html#download-token)，deadline 是 dntoken 的有效期。dntoken不需要单独生成，C-SDK 提供了生成完整 downloadUrl 的方法（包含了 dntoken），示例代码如下：

```{c}
char* downloadUrl(Qiniu_Client* client, const char* domain, const char* key)
{
	char* url;
	char* baseUrl;
	Qiniu_RS_GetPolicy getPolicy;

	Qiniu_Zero(getPolicy);
	baseUrl = Qiniu_RS_MakeBaseUrl(domain, key); // baseUrl也可以自己拼接："http://"+domain+"/"+urlescape(key)
	url = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, baseUrl, NULL);

	Qiniu_Free(baseUrl);
	return url;                                  // When url is no longer being used, free it by Qiniu_Free.
}
```

生成 downloadUrl 后，服务端下发 downloadUrl 给客户端。客户端收到 downloadUrl 后，和公有资源类似，直接用任意的 HTTP 客户端就可以下载该资源了。唯一需要注意的是，在 downloadUrl 失效却还没有完成下载时，需要重新向服务器申请授权。

无论公有资源还是私有资源，下载过程中客户端并不需要七牛 C-SDK 参与其中。

<a name="io-https-get"></a>

### HTTPS 支持

几乎所有七牛云存储 API 都同时支持 HTTP 和 HTTPS，但 HTTPS 下载有些需要注意的点。如果你的资源希望支持 HTTPS 下载，有如下限制：

1. 不能用 xxx.qiniudn.com 这样的二级域名，只能用 dn-xxx.qbox.me 域名。样例：https://dn-abc.qbox.me/1.txt
2. 使用自定义域名是付费的。我们并不建议使用自定义域名，但如确有需要，请联系我们的销售人员。

<a name="resumable-io-get"></a>

### 断点续下载

无论是公有资源还是私有资源，获得的下载 url 支持标准的 HTTP 断点续传协议。考虑到多数语言都有相应的断点续下载支持的成熟方法，七牛 C-SDK 并不提供断点续下载相关代码。


<a name="rs"></a>

## 资源操作

资源操作包括对存储在七牛云存储上的文件进行查看、删除、复制和移动处理。同时七牛云存储也支持对文件进行相应的批量操作。
所有操作都会返回一个`Qiniu_Error`的结构体，用于记录该次操作的成功/失败信息。

```{c}
typedef struct _Qiniu_Error {
	int code;
	const char* message;
} Qiniu_Error;
```

<a name="rs-stat"></a>

### 获取文件信息

```{c}
void stat(Qiniu_Client* client, const char* bucket, const char* key)
{
	Qiniu_RS_StatRet ret;
	Qiniu_Error err = Qiniu_RS_Stat(client, &ret, bucket, key);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("hash: %s, fsize: %lld, mimeType: %s\n", ret.hash, ret.fsize, ret.mimeType);
}
```

通过调用`Qiniu_RS_Stat`，可以得到指定文件的属性信息。除了会返回一个`Qiniu_Error`结构体之外，`Qiniu_RS_Stat`还会返回`Qiniu_RS_StatRet`这个结构体，其中记录了被查询文件的一些属性信息。

```{c}
typedef struct _Qiniu_RS_StatRet {
	const char* hash;
	const char* mimeType;
	Qiniu_Int64 fsize;	
	Qiniu_Int64 putTime;
} Qiniu_RS_StatRet;
```

<a name="rs-delete"></a>

### 删除文件

调用`Qiniu_RS_Delete`并指定bucket和key，即可完成对一个文件的删除操作，同样`Qiniu_Error`结构体中记录了成功/失败信息。

```{c}
void delete(Qiniu_Client* client, const char* bucket, const char* key)
{
	Qiniu_Error err = Qiniu_RS_Delete(client, bucket, key);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("%s:%s delete OK.\n", bucket, key);
}
```
<a name="rs-copy-move"></a>

### 复制/移动文件

复制和移动操作，需要指定源路径和目标路径。

```{c}
void copy(Qiniu_Client* client, 
	const char* bucketSrc, const char* keySrc, 
	const char* bucketDest, const char* keyDest)
{
	Qiniu_Error err = Qiniu_RS_Copy(client, bucketSrc, keySrc, bucketDest, keyDest);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("Copy %s:%s -> %s:%s OK.\n", bucketSrc, keySrc, bucketDest, keyDest);
}
```

```{c}
void move(Qiniu_Client* client, 
	const char* bucketSrc, const char* keySrc, 
	const char* bucketDest, const char* keyDest)
{
	Qiniu_Error err = Qiniu_RS_Move(client, bucketSrc, keySrc, bucketDest, keyDest);
	if (err.code != 200) {
		debug(client, err);
		return;
	}
	printf("Move %s:%s -> %s:%s OK.\n", bucketSrc, keySrc, bucketDest, keyDest);
}
```

<a name="rs-batch"></a>

### 批量操作

在支持单一的资源操作的同时，七牛云存储还支持批量地进行查看、删除、复制和移动操作。

#### 批量查看

调用`Qiniu_RS_BatchStat`可以批量查看多个文件的属性信息。

```{c}
void batchStat(Qiniu_Client* client, 
	Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchStatRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchStatRet));
	Qiniu_Error err = Qiniu_RS_BatchStat(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		} else {
			printf("hash: %s\n", rets[curr].data.hash);
			printf("mimeType: %s\n", rets[curr].data.mimeType);
			printf("fsize: %lld\n", rets[curr].data.fsize);
			printf("putTime: %lld\n", rets[curr].data.putTime);
		}
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
```

其中，`entries`是一个指向`Qiniu_RS_EntryPath`结构体数组的指针，`entryCount`为数组`entries`的长度。结构体`Qiniu_RS_EntryPath`中填写每个文件相应的bucket和key：

```{c}
typedef struct _Qiniu_RS_EntryPath {
    const char* bucket;
    const char* key;
} Qiniu_RS_EntryPath;
```

`Qiniu_RS_BatchStat`会将文件信息（及成功/失败信息）依次写入一个由结构体`Qiniu_RS_BatchStatRet`组成的数组空间`rets`。因此，调用之前，需要先给`rets`申请好相应长度的内存空间。

其中结构体`Qiniu_RS_BatchStatRet`的组成如下：

```{c}
typedef struct _Qiniu_RS_BatchStatRet {
    Qiniu_RS_StatRet data;
    const char* error;
    int code;
}Qiniu_RS_BatchStatRet;
```

结构体`Qiniu_RS_StatRet`的组成为：

```{c}
typedef struct _Qiniu_RS_StatRet {
	const char* hash;
	const char* mimeType;
	Qiniu_Int64 fsize;	
	Qiniu_Int64 putTime;
} Qiniu_RS_StatRet;
```

需要注意的是，通过动态内存申请得到的内存空间在使用完毕后应该立即释放。

#### 批量删除

调用`Qiniu_RS_BatchDelete`可以批量删除多个文件。

```{c}
void batchDelete(Qiniu_Client* client, 
	Qiniu_RS_EntryPath* entries, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchDelete(client, rets, entries, entryCount);

	int curr = 0;
	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		}
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
```

和批量查看一样，`entries`是一个指向`Qiniu_RS_EntryPath`结构体数组的指针，`entryCount`为数组`entries`的长度。`Qiniu_RS_BatchDelete`会将删除操作的成功/失败信息依次写入一个由结构体`Qiniu_RS_BatchItemRet`组成的数组空间`rets`。同样需要先申请好相应长度的内存空间。

其中结构体`Qiniu_RS_BatchItemRet`的组成如下：

```{c}
typedef struct _Qiniu_RS_BatchItemRet {
    const char* error;
    int code;
}Qiniu_RS_BatchItemRet;
```

#### 批量复制

调用`Qiniu_RS_BatchCopy`可以批量复制多个文件。

```{c}
void batchCopy(Qiniu_Client* client, 
	Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchCopy(client, rets, entryPairs, entryCount);
	int curr = 0;

	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		}
		curr++;
	}
	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
```

批量复制需要指明每个操作的源路径和目标路径，`entryPairs`是一个指向`Qiniu_RS_EntryPathPair`结构体数组的指针，`entryCount`为数组`entryPairs`的长度。结构体`Qiniu_RS_EntryPathPair`结构如下：

```{c}
typedef struct _Qiniu_RS_EntryPathPair {
    Qiniu_RS_EntryPath src;
    Qiniu_RS_EntryPath dest;
} Qiniu_RS_EntryPathPair;
```

同之前一样 ，`Qiniu_RS_BatchCopy`会将复制操作的成功/失败信息依次写入一个由结构体`Qiniu_RS_BatchItemRet`组成的数组空间`rets`。

#### 批量移动

批量移动和批量复制很类似，唯一的区别就是调用`Qiniu_RS_BatchMove`。

```{c}
void batchMove(Qiniu_Client* client, 
	Qiniu_RS_EntryPathPair* entryPairs, Qiniu_ItemCount entryCount)
{
	Qiniu_RS_BatchItemRet* rets = calloc(entryCount, sizeof(Qiniu_RS_BatchItemRet));
	Qiniu_Error err = Qiniu_RS_BatchMove(client, rets, entryPairs, entryCount);

	int curr = 0;
	while (curr < entryCount) {
		printf("\ncode: %d\n", rets[curr].code);

		if (rets[curr].code != 200) {
			printf("error: %s\n", rets[curr].error);
		}
		curr++;
	}

	free(rets);

	if (err.code != 200) {
		debug(client, err);
		return;
	}
}
```

