#include <stdio.h>
#include <stdlib.h>
#include "../qiniu/rs.h"
#include "../qiniu/io.h"
#include "../qiniu/http.h"

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

	err = Qiniu_Io_PutFile(&cli, &ret, uptoken, key, localFile, &extra);
	free((void*)uptoken);
	free((void*)pp.scope);
	printf("code=%d msg=[%s]\n", err.code, err.message);

	Qiniu_Client_Cleanup(&cli);
	Qiniu_MacAuth_Cleanup();
	Qiniu_Servend_Cleanup();
	Qiniu_Global_Cleanup();

	if (err.code != 200) {
		return 1;
	} // if

	return 0;
} // main
