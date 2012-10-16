#include "qbox_test.h"
#include "../../qbox/rs.h"

int mkbucket(QBox_Client* client)
{
	QBox_Error err;

	QBox_RS_Drop(client, TEST_TABLE);

	err = QBox_RS_Create(client, TEST_TABLE);
	if (err.code != 200) {
		QBT_Fatalfln("code:%d, msg:%s", err.code, err.message);
	}

	return 0;
}
