#include "qbox_test.h"
#include "../../qbox/image.h"
#include "../../qbox/rs.h"

#define ImgURL "http://qiniuphotos.dn.qbox.me/gogopher.jpg"

int saveas(QBox_Client* client)
{
	QBox_Error err;
	QBox_IMG_SaveAsRet ret;
	QBox_IMG_MogrOpts opts;
	QBox_IMG_InitMogrOpts(&opts);

	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save1");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}
	
	opts.auto_orient = 1;
	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save2");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	opts.quality = "60";
	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save3");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	opts.thumbnail = "!120x120r";
	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save4");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	opts.gravity = "North";
	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save5");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	opts.crop = "!300x400a10a10";
	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save6");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	opts.rotate = "45";
	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save7");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	opts.format = "png";
	err = QBox_IMG_SaveAs(client, &ret, &opts, ImgURL, "c-test", "save8");
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	return 0;
}
