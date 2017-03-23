#include "../qiniu/cdn.h"
#include "../qiniu/tm.h"

int main(int argc, char * argv[])
{
    char * key = argv[1];
    char * path = argv[2];
    char * deadlineStr = argv[3];
    char * encodedPath;
    Qiniu_Uint64 deadline;

    if (argc < 3) {
        printf("Usage: gen_download_url_with_deadline <KEY> <URL> [DEADLINE]\n");
        return 1;
    } // if

    if (deadlineStr) {
        deadline = atol(deadlineStr);
    } else {
        deadline = Qiniu_Tm_LocalTime() + 3600;
    } // if

    encodedPath = Qiniu_Cdn_MakeDownloadUrlWithDeadline(key, path, deadline);
    printf("%s\n", encodedPath);
    Qiniu_Free(encodedPath);
    return 0;
}
