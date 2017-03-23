#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../qiniu/rs.h"
#include "../qiniu/io.h"
#include "../qiniu/http.h"

size_t rdr(char* buffer,size_t size,size_t n,void* fp)
{
    size_t nread = fread(buffer,size,n,fp);
    if (nread==0) printf("READ DONE.\n");
    return nread;
}

int main(int argc, char * argv[])
{
	Qiniu_Error err;
	Qiniu_Client cli;
	Qiniu_Mac mac;
	Qiniu_Io_PutExtra extra;
	Qiniu_Io_PutRet ret;
	Qiniu_RS_PutPolicy pp;
	const char * uptoken = NULL;
	const char * bucket = NULL;
	const char * key = NULL;
	const char * localFile = NULL;

	Qiniu_Global_Init(0);
	Qiniu_Servend_Init(0);
	Qiniu_MacAuth_Init();

	memset(&extra, 0, sizeof(extra));
	memset(&pp, 0, sizeof(pp));

	mac.accessKey = argv[1];
	mac.secretKey = argv[2];
	bucket = argv[3];
	key = argv[4];
	localFile = argv[5];

	if (argc >= 7) {
		extra.upHost = argv[6];
	} // if

	pp.scope = Qiniu_String_Format(512, "%s:%s", bucket, key);
	uptoken = Qiniu_RS_PutPolicy_Token(&pp, &mac);

	Qiniu_Client_InitNoAuth(&cli, 8192);

    struct stat fi;
    stat(localFile, &fi);
    size_t fsize = fi.st_size;
    printf("fsize:%d\n",fsize);

    FILE* fp = fopen(localFile,"rb");

    err = Qiniu_Io_PutStream(&cli,&ret,uptoken,key,fp,fsize,rdr,&extra);

    fclose(fp);

	free((void*)uptoken);
	free((void*)pp.scope);

	printf("code:%d\nmsg:[%s]\n", err.code, err.message);

	Qiniu_Client_Cleanup(&cli);
	Qiniu_MacAuth_Cleanup();
	Qiniu_Servend_Cleanup();
	Qiniu_Global_Cleanup();

	return 0;
} // main
