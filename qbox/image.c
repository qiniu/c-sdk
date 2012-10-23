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
		QBox_Client* self, QBox_IMG_InfoRet* ret, const char* imgURL)
{
	QBox_Error err;
	cJSON* root;

	char* url = QBox_String_Concat2(imgURL, "?imageInfo");

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
/* func QBox_IMG_Exif, QBox_IMG_ExifRet_Release */

QBox_Error QBox_IMG_Exif(
		QBox_Client* client, QBox_IMG_ExifRet* ret, const char* imgURL)
{
	cJSON* root;
	cJSON* array;
	cJSON* object;
	QBox_Error err;
	QBox_Uint32 index;

	char* url = QBox_String_Concat2(imgURL, "?exif");

	err = QBox_Client_Call(client, &root, url);
	free(url);

	if (err.code == 200) {
		array = root->child;
		ret->size = cJSON_GetArraySize(root);
		ret->info = (QBox_IMG_ExifInfo*)malloc(ret->size*sizeof(QBox_IMG_ExifInfo));
		index = 0;
		while (array != NULL) {
			ret->info[index].name = array->string;
			ret->info[index].val = "";
			ret->info[index].type = 0;
			object = array->child;
			if (object != NULL) {
				ret->info[index].val = object->valuestring;
				object = object->next;
				if (object != NULL) {
					ret->info[index].type = (QBox_Int64)object->valuedouble;
				}
			}
			index++;
			array = array->next;
		}
	}
	return err;
}

QBox_Error QBox_IMG_ExifRet_Release(QBox_IMG_ExifRet ret)
{
	free(ret.info);
}

/*============================================================================*/
/* func QBox_IMG_InitViewOpts, QBox_IMG_ViewURL */

void QBox_IMG_InitViewOpts(QBox_IMG_ViewOpts* opts)
{
	memset(opts, 0, sizeof(QBox_IMG_ViewOpts));
	opts->width = -1;
	opts->height = -1;
	opts->quality = -1;
	opts->sharpen = -1;
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

char* _dirtycat2n(char* dst, const char* src1st, int src2nd)
{
	char buffer[12] = { 0 };
	if (src2nd < 0) {
		return dst;
	}

	sprintf(buffer, "%d", src2nd);
	return _dirtycat2(dst, src1st, buffer);
}

// remember to free the returned pointer when not needed anymore.
char* QBox_IMG_ViewURL(QBox_IMG_ViewOpts* opts, const char* imgURL)
{
	char* ret = NULL;
	char* view = "?imageView";
	int urllen = strlen(imgURL);
	int viewlen = strlen(view);

	ret = (char*)calloc(urllen + viewlen + 1, sizeof(char));

	strcpy(ret, imgURL);
	strcat(ret, view);

	ret = _dirtycat2n(ret, "/", opts->mode);
	ret = _dirtycat2n(ret, "/w/", opts->width);
	ret = _dirtycat2n(ret, "/h/", opts->height);
	ret = _dirtycat2n(ret, "/q/", opts->quality);
	ret = _dirtycat2(ret, "/format/", opts->format);
	ret = _dirtycat2n(ret, "/sharpen/", opts->sharpen);
	if (opts->watermark != 0) {
		ret = _dirtycat2n(ret, "/watermark/", opts->watermark);
	}

	return ret;
}

/*============================================================================*/
/* func QBox_IMG_InitMogrOpts, QBox_IMG_MogrifyUrl */

void QBox_IMG_InitMogrOpts(QBox_IMG_MogrOpts* opts)
{
	memset(opts, 0, sizeof(QBox_IMG_MogrOpts));
}

char* QBox_IMG_MogrifyURL(QBox_IMG_MogrOpts* opts, const char* imgURL)
{
	char* ret = NULL;
	char* mogr = "?imageMogr";
	int urllen = strlen(imgURL);
	int mogrlen = strlen(mogr);

	ret = (char*)calloc(urllen + mogrlen + 1, sizeof(char));

	strcpy(ret, imgURL);
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
		QBox_IMG_MogrOpts* opts, const char* imgURL,
		const char* tableName, const char* key)
{
	QBox_Error err;
	cJSON* root;

	char* entryURI = QBox_String_Concat3(tableName, ":", key);
	char* entryURIEncoded = QBox_String_Encode(entryURI);
	char* mogrURL = QBox_IMG_MogrifyURL(opts, imgURL);
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

