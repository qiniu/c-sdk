#include "qbox_test.h"
#include "../../qbox/rs.h"

static const char* s_table = "c-test";

int mkbucket(QBox_Client* client)
{
	QBox_Error err;

	QBox_RS_Drop(client, s_table);

	err = QBox_RS_Create(client, s_table);
	if (err.code != 200) {
		QBT_Fatalfln("create failed => code:%d, msg:%s", err.code, err.message);
	}

	return 0;
}
