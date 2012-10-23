#include "qbox_test.h"
#include "../../qbox/image.h"
#include "../../qbox/rs.h"

#define ImgURL "http://qiniuphotos.dn.qbox.me/gogopher.jpg"

int saveas(QBox_Client* client)
{
	QBox_Error err;
	QBox_RS_GetRet getRet;
	QBox_RS_PutRet putRet;
	QBox_IMG_SaveAsRet ret;
	QBox_IMG_MogrOpts opts;
	QBox_IMG_InitMogrOpts(&opts);
	char* imgURL = NULL;
	
	err = QBox_RS_PutFile(client, &putRet, TABLE, "gogopher.jpg", "image/jpeg", "gogopher.jpg", "");
	QBT_CheckErr(err);

	err = QBox_RS_Get(client, &getRet, TABLE, "gogopher.jpg", NULL);
	QBT_CheckErr(err);

	imgURL = (char*)malloc(strlen(getRet.url)*sizeof(char));
	strcpy(imgURL, getRet.url);

	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save1");
	QBT_CheckErr(err);
	
	opts.auto_orient = 1;
	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save2");
	QBT_CheckErr(err);

	opts.quality = "60";
	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save3");
	QBT_CheckErr(err);

	opts.thumbnail = "!120x120r";
	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save4");
	QBT_CheckErr(err);

	opts.gravity = "North";
	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save5");
	QBT_CheckErr(err);

	opts.crop = "!300x400a10a10";
	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save6");
	QBT_CheckErr(err);

	opts.rotate = "45";
	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save7");
	QBT_CheckErr(err);

	opts.format = "png";
	err = QBox_IMG_SaveAs(client, &ret, &opts, imgURL, TABLE, "save8");
	QBT_CheckErr(err);

	free(imgURL);

	return 0;
}
