#include <curl/curl.h>
#include "private/code.h"

#define CURLE_WEIRD_SERVER_REPLY 8
#define CURLE_HTTP2_STREAM 92
#define CURLE_HTTP3 95
#define CURLE_QUIC_CONNECT_ERROR 96

Qiniu_Retry_Decision _Qiniu_Should_Retry(int code)
{
    if (code / 100 == 4 && code != 406 && code != 429)
    {
        return QINIU_DONT_RETRY;
    }
    switch (code)
    {
    case CURLE_COULDNT_RESOLVE_HOST:
    case CURLE_COULDNT_CONNECT:
    case CURLE_WEIRD_SERVER_REPLY:
    case CURLE_PARTIAL_FILE:
    case CURLE_UPLOAD_FAILED:
    case CURLE_OPERATION_TIMEDOUT:
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SEND_ERROR:
    case CURLE_RECV_ERROR:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_PEER_FAILED_VERIFICATION:
    case CURLE_HTTP2_STREAM:
    case CURLE_HTTP3:
    case CURLE_QUIC_CONNECT_ERROR:
    case 406:
    case 429:
    case 500:
    case 502:
    case 503:
    case 504:
    case 509:
    case 571:
    case 573:
    case 599:
        return QINIU_TRY_NEXT_DOMAIN;

    default:
        return QINIU_DONT_RETRY;
    }
}
