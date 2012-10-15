/*
 ============================================================================
 Name        : image.c
 Author      : Jiang WenLong
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#include "image.h"

/*============================================================================*/
/* func QBox_IMG_Info */

QBox_Error QBox_IMG_Info(
		QBox_Client* self, QBox_IMG_InfoRet* ret, const char* imgUrl)
{
	QBox_Error err;
	cJSON* root;

	char* url = QBox_String_Concat2(imgUrl, "/imageInfo");

	err = QBox_Client_Call(self, &root, url);
	free(url);

	if (err.code == 200) {
		ret->format = QBox_Json_GetString(root, "format", NULL);
		ret->colorModel = QBox_Json_GetString(root, "colorModel", NULL);
		ret->width = QBox_Json_GetInt64(root, "width", 0);
		ret->height = QBox_Json_GetInt64(root, "height", 0);
	}
	return err;
}

/*============================================================================*/
/* func QBox_IMG_MogrifyUrl */

void QBox_IMG_InitMogrOpts(QBox_IMG_MogrOpts* opts)
{
	memset(opts, 0, sizeof(QBox_IMG_MogrOpts));
}

static char* _dirtycat(char* dst, const char* src)
{
	int dstlen = 0;
	int srclen = 0;
	if (src == NULL || src[0] == '\0') {
		return dst;
	}

	dstlen = strlen(dst);
	srclen = strlen(src);
	dst = (char*)realloc(dst, dstlen + srclen + 1);
	memcpy(dst + dstlen, src, srclen);
	dst[dstlen + srclen] = '\0';

	return dst;
}

static char* _dirtycat2(char* dst, const char* src1st, const char* src2nd)
{
	int dstlen = 0;
	int src1stlen = 0;
	int src2ndlen = 0;
	if (src1st == NULL || src1st[0] == '\0' || src2nd == NULL || src2nd[0] == '\0') {
		return dst;
	}

	dstlen = strlen(dst);
	src1stlen = strlen(src1st);
	src2ndlen = strlen(src2nd);
	dst = (char*)realloc(dst, dstlen + src1stlen + src2ndlen+ 1);
	memcpy(dst + dstlen, src1st, src1stlen);
	memcpy(dst + dstlen + src1stlen, src2nd, src2ndlen);
	dst[dstlen + src1stlen + src2ndlen] = '\0';

	return dst;
}

char* QBox_IMG_MogrifyUrl(QBox_IMG_MogrOpts* opts, const char* url)
{
	char* ret = NULL;
	char* mogr = "?imageMogr";
	int urllen = strlen(url);
	int mogrlen = strlen(url);

	ret = (char*)calloc(urllen + mogrlen + 1, sizeof(char));

	strcpy(ret, url);
	strcat(ret, mogr);

	ret = _dirtycat2(ret, "/thumbnail/", opts->thumbnail);
	ret = _dirtycat2(ret, "/gravity/", opts->gravity);
	ret = _dirtycat2(ret, "/crop/", opts->crop);
	ret = _dirtycat2(ret, "/quality/", opts->quality);
	ret = _dirtycat2(ret, "/rotate/", opts->rotate);
	ret = _dirtycat2(ret, "/format/", opts->format);
	if (opts->auto_orient) {
		ret = _dirtycat(ret, "/auto-orient");
	}

	return ret;
}

/*============================================================================*/
/* func QBox_IMG_SaveAs */

QBox_Error QBox_IMG_SaveAs(QBox_Client* self, QBox_IMG_SaveAsRet* ret, 
		QBox_IMG_MogrOpts* opts, const char* url,
		const char* tableName, const char* key)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* mogrURL = QBox_IMG_MogrifyUrl(opts, url);
	char* saveURL = QBox_String_Concat3(mogrURL, "/save-as/", entryURIEncoded);

	err = QBox_Client_Call(self, &root, saveURL);

	free(saveURL);
	free(mogrURL);
	free(entryURIEncoded);
	free(entryURI);

	if (err.code == 200) {
		ret->hash = QBox_Json_GetString(root, "hash", NULL);
	}
	return err;
}

