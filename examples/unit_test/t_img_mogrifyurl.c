#include "qbox_test.h"
#include "../../qbox/image.h"

#define ImgURL "http://qiniuphotos.dn.qbox.me/gogopher.jpg"

int mogrifyurl(QBox_Client* client)
{
	char* url = NULL;
	QBox_IMG_MogrOpts opts;
	QBox_IMG_InitMogrOpts(&opts);

	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	opts.auto_orient = 1;
	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr/auto-orient") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	opts.quality = "60";
	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr/quality/60/auto-orient") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	opts.thumbnail = "50%";
	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr/thumbnail/50%/quality/60/auto-orient") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	opts.gravity = "North";
	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr/thumbnail/50%/gravity/North/quality/60/auto-orient") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	opts.crop = "!300x400a10a10";
	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/auto-orient") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	opts.rotate = "45";
	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/rotate/45/auto-orient") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	opts.format = "png";
	url = QBox_IMG_MogrifyURL(&opts, ImgURL);
	QBT_Infof("%s\n", url);
	if (strcmp(url, ImgURL "?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/rotate/45/format/png/auto-orient") != 0) {
		free(url);
		QBT_Fatalf("unexpected url!\n");
	}
	free(url);

	return 0;
}

