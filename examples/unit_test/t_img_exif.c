#include "qbox_test.h"
#include "../../qbox/image.h"

static char* g_imgURL = "http://qiniuphotos.dn.qbox.me/gogopher.jpg";

int exif(QBox_Client* client)
{
	QBox_Error err;
	QBox_Uint32 index;
	QBox_IMG_ExifRet ret;

	err = QBox_IMG_Exif(client, &ret, g_imgURL);
	QBT_CheckErr(err);

	for (index = 0; index < ret.size; index++) {
		QBT_Infof("name: %-25s val: %-20s type: %d\n", 
				ret.info[index].name,
				ret.info[index].val,
				ret.info[index].type);
	}

	QBox_IMG_ExifRet_Release(ret);

	return 0;
}
