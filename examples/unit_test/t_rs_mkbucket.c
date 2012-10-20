#include "qbox_test.h"
#include "../../qbox/rs.h"

int mkbucket(QBox_Client* client)
{
	QBox_Error err;

	QBox_RS_Drop(client, TABLE);

	err = QBox_RS_Create(client, TABLE);
	QBT_CheckErr(err);

	return 0;
}
