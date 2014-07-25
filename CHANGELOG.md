## CHANGE LOG

### v6.2.4

- 单元测试调整

### v6.2.3

- 调整上传host 

### v6.2.2

2013-10-14 issue [#112](https://github.com/qiniu/c-sdk/pull/112) [#114](https://github.com/qiniu/c-sdk/pull/114) [#116](https://github.com/qiniu/c-sdk/pull/116)

- 解决几个内存泄漏


### v6.2.1

2013-07-25 issue [#102](https://github.com/qiniu/c-sdk/pull/102), [#104](https://github.com/qiniu/c-sdk/pull/104)

- struct pack(1)
- 中文编码问题：visual c++ 2010 由于把 utf8 误当 gbk，并且处理不当，导致代码被当做注释
  - 暂且把代码中的所有中文注释去掉


### v6.2.0

2013-07-25 issue [#99](https://github.com/qiniu/c-sdk/pull/99)

- resumable io (for windows) bugfix
  - see [Qiniu_Posix_Pread](https://github.com/qiniu/c-sdk-wdeps/commit/5446ccdef5f4f695aec5bc3d4b68987e78b2fdfc)


### v6.1.2

2013-07-06 issue [#93](https://github.com/qiniu/c-sdk/pull/93)

- 支持批处理（Batch）
- 支持 Move/Copy


### v6.1.1

2013-07-02 issue [#91](https://github.com/qiniu/c-sdk/pull/91)

- bugfix: vc++ 6.0 sprintf doesn't support %zu


### v6.1.0

2013-07-01 issue [#87](https://github.com/qiniu/c-sdk/pull/87)

- bugfix: add Qiniu_Free for qiniu.dll (required by windows dll memory management)


### v6.0.1

2013-06-30 issue [#79](https://github.com/qiniu/c-sdk/pull/79)

- 遵循 [sdkspec v6.0.2](https://github.com/qiniu/sdkspec/tree/v6.0.2)
  - io.Put/PutFile 调整为基于 up.qiniu.com 的协议，extra *PutExtra 参数可以为 NULL
  - io.Put/PutFile 支持支持 key = NULL (UNDEFINED_KEY)，这样服务端将自动生成 key 并返回
  - io.Put/PutFile 支持自定义的 "x:" 参数(io.PutExtra.Params)
  - io.Put/PutFile 暂未支持 Crc、MimeType
  - 新的 PutPolicy 结构


### v6.0.0

2013-06-26 issue [#76](https://github.com/qiniu/c-sdk/pull/76)

- 增加 `Qiniu_PathEscape`
- 遵循 [sdkspec v6.0.0](https://github.com/qiniu/sdkspec/tree/v6.0.0)
  - `Qiniu_RS_GetPolicy_Token` => `Qiniu_RS_GetPolicy_MakeRequest`
  - 增加 `Qiniu_RS_MakeBaseUrl`
  - 增加 `Qiniu_Mac_Sign`


### v5.2.0

2013-05-26 issue [#69](https://github.com/qiniu/c-sdk/pull/69)

- 支持 Windows 平台（VC++6.0 tested）。


### v5.1.0

2013-04-29 issue [#54](https://github.com/qiniu/c-sdk/pull/54), [#59](https://github.com/qiniu/c-sdk/pull/59)

- 增加断点续上传支持 (Qiniu_Rio_Put/PutFile)。
- 补充了大量C语言基础组件 (StringFormat, Logger, Copy, TeeReader, SectionReader, Crc32Writer, etc)
- 非兼容调整（细节）：Qiniu_Client_Init 改名为 Qiniu_Client_InitMacAuth，以明其义。
- 非兼容调整（细节）：Qiniu_RS_GetPolicy_Token, Qiniu_RS_PutPolicy_Token 增加参数 Qiniu_Mac* mac。
- 引入 Qiniu_Servend_Init/Cleanup，在服务端用的时候应该用它们而不是 Qiniu_Global_Init/Cleanup。


### v5.0.1

2013-04-22 issue [#41](https://github.com/qiniu/c-sdk/pull/41)

- 补充 v5.0.0 的 SDK 文档。


### v5.0.0

2013-04-21 issue [#39](https://github.com/qiniu/c-sdk/pull/39)

- 非兼容调整。完全重构，遵循 [Qiniu sdkspec](https://github.com/qiniu/sdkspec)。


### v2.2.2

2013-04-20 issue [#36](https://github.com/qiniu/c-sdk/pull/36)

- 增加 QBox_RS_PutPolicy_Token, QBox_RS_GetPolicy_Token (不建议继续使用 QBox_MakeUpToken)


### v2.2.1

2013-04-19 issue [#24](https://github.com/qiniu/c-sdk/pull/24)

- 增加 QBox_Client_InitNoAuth
- 增加 QBox_Io_PutFile, QBox_Io_PutBuffer
- 增加 QBox_RS_PutStream, QBox_RSCli_PutStream, QBox_RSCli_UploadStream (但不推荐，推荐 QBox_Io_PutXXX)
- 支持 Travis-CI，引入 CUnit 做单元测试

