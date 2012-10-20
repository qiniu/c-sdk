#include "qbox_test.h"
#include "../../qbox/image.h"

#define IMG_URL "http://qiniuphotos.dn.qbox.me/gogopher.jpg"

int viewurl(QBox_Client* client)
{
	char* url = NULL;
	QBox_IMG_ViewOpts opts;
	QBox_IMG_InitViewOpts(&opts);

	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/0") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);

	opts.mode = 1;
	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/1") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);

	opts.width = 100;
	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/1/w/100") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);

	opts.height = 200;
	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/1/w/100/h/200") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);

	opts.quality = 50;
	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/1/w/100/h/200/quality/50") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);

	opts.format = "png";
	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/1/w/100/h/200/quality/50/format/png") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);
	
	opts.sharpen = 60;
	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/1/w/100/h/200/quality/50/format/png/sharpen/60") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);

	opts.watermark = 1;
	url = QBox_IMG_ViewURL(&opts, IMG_URL);
	QBT_Infof("url:%s\n", url);
	if (strcmp(url, IMG_URL "?imageView/1/w/100/h/200/quality/50/format/png/sharpen/60/watermark/1") != 0) {
		free(url);
		QBT_Fatalf("unexpected!\n");
	}
	free(url);

	return 0;
}
