/*
 ============================================================================
 Name        : image.c
 Author      : Jiang WenLong
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

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
