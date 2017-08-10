#include "../qiniu/cdn.h"
#include "../qiniu/tm.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    char *host = "http://v1.cdn.example.com";
    char *fileName = "video/caipu/201708/0600/s/5985f37b37025.mp4/eGgxMDgwcA.m3u8";
    char *cryptKey = "xxx";
    char *queryStr = "name=七牛&age=27";

    char *finalUrl;
    Qiniu_Uint64 deadline = Qiniu_Tm_LocalTime() + 3600;

    finalUrl = Qiniu_CDN_CreateTimestampAntiLeechURL(host, fileName, queryStr, deadline, cryptKey);
    printf("%s\n", finalUrl);
    Qiniu_Free(finalUrl);
}
