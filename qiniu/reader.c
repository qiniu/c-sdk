#include <curl/curl.h>

#include "reader.h"

QINIU_DLLAPI Qiniu_Error Qiniu_Rd_Reader_Open(Qiniu_Rd_Reader * rdr, const char * localFileName)
{
	Qiniu_Error err;
	err = Qiniu_File_Open(&rdr->file, localFileName);
	if (err.code != 200) {
		return err;
	} // if

	rdr->offset = 0;
	rdr->status = QINIU_RD_OK;

	return Qiniu_OK;
}

QINIU_DLLAPI void Qiniu_Rd_Reader_Close(Qiniu_Rd_Reader * rdr)
{
	Qiniu_File_Close(rdr->file);
}

QINIU_DLLAPI size_t Qiniu_Rd_Reader_Callback(char * buffer, size_t size, size_t nitems, void * userData)
{
	ssize_t ret;
	Qiniu_Rd_Reader * rdr = (Qiniu_Rd_Reader *)userData;

	ret = Qiniu_File_ReadAt(rdr->file, buffer, size * nitems, rdr->offset);
	if (ret < 0) {
		rdr->status = QINIU_RD_ABORT_BY_READAT;
		return CURL_READFUNC_ABORT;
	} // if

	if (rdr->abortCallback && rdr->abortCallback(rdr->abortUserData, buffer, ret)) {
		rdr->status = QINIU_RD_ABORT_BY_CALLBACK;
		return CURL_READFUNC_ABORT;
	} // if

	rdr->offset += ret;
	return ret;
}
