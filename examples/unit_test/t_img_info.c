#include "qbox_test.h"
#include "../../qbox/image.h"

static char* g_imgURL = "http://qiniuphotos.dn.qbox.me/gogopher.jpg";

int imginfo(QBox_Client* client)
{
	QBox_Error err;
	QBox_IMG_InfoRet ret;

	err = QBox_IMG_Info(client, &ret, g_imgURL);
	QBT_CheckErr(err);

	QBT_Infof("format: %s\n", ret.format);
	QBT_Infof("colorModel: %s\n", ret.colorModel);
	QBT_Infof("width: %d\n", ret.width);
	QBT_Infof("height: %d\n", ret.height);

	return 0;
}
