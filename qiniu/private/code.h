typedef enum
{
    QINIU_DONT_RETRY,
    QINIU_TRY_NEXT_DOMAIN
} Qiniu_Retry_Decision;

Qiniu_Retry_Decision _Qiniu_Should_Retry(int code);
