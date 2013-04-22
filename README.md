# Qiniu Resource (Cloud) Storage SDK for C/C++

[![Build Status](https://api.travis-ci.org/qiniu/c-sdk.png?branch=master)](https://travis-ci.org/qiniu/c-sdk)

[![Qiniu Logo](http://qiniutek.com/images/logo-2.png)](http://qiniu.com/)

此 C/C++ SDK 适用于 兼容C89标准的C/C++编译器，基于 [七牛云存储官方API](http://docs.qiniutek.com/v2/api/) 构建。使用此 SDK 构建您的网络应用程序，能让您以非常便捷地方式将数据安全地存储到七牛云存储上。无论您的网络应用是一个网站程序，还是包括从云端（服务端程序）到终端（手持设备应用）的架构的服务或应用，通过七牛云存储及其 SDK，都能让您应用程序的终端用户高速上传和下载，同时也让您的服务端更加轻盈。

## 编译

在您 C/C++ 应用程序的 工程文件中，添加本SDK提供的各个头文件和源文件;
同时在需要调用API的各个源工件中加入对oauth2.h和rs.h的包含指令。

## 使用

参考文档：[七牛云存储 C SDK 使用指南](https://github.com/qiniu/c-sdk/tree/v5/docs)

## 贡献代码

1. Fork
2. 创建您的特性分支 (`git checkout -b my-new-feature`)
3. 提交您的改动 (`git commit -am 'Added some feature'`)
4. 将您的修改记录提交到远程 `git` 仓库 (`git push origin my-new-feature`)
5. 然后到 github 网站的该 `git` 远程仓库的 `my-new-feature` 分支下发起 Pull Request

## 许可证

Copyright (c) 2012 qiniutek.com

基于 MIT 协议发布:

* [www.opensource.org/licenses/MIT](http://www.opensource.org/licenses/MIT)
