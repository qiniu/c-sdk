#include "wm.h"
#include "oauth2.h"
#include <stdio.h>

/*============================================================================*/
/* func QBox_WM_Get, QBox_WM_Set */

QBox_Error QBox_WM_Get( QBox_Client* self, char *customer, QBox_WM_Template *tpl )
{
	QBox_Error err;
	cJSON* root;

	char* url = QBox_String_Concat(QBOX_EU_HOST, "/wmget", NULL);
	char* body = QBox_String_Concat("customer=", customer, NULL);

	err = QBox_Client_CallWithForm(self, &root, url, body, strlen(body));
	free(url);
	free(body);
	tpl->Font = (char *)QBox_Json_GetString(root,"font",NULL);
	tpl->PointSize = QBox_Json_GetInt64(root,"fontsize",12);
	tpl->Fill = (char *)QBox_Json_GetString(root,"fill",NULL);
	tpl->Text = (char *)QBox_Json_GetString(root,"text",NULL);
	tpl->Bucket = (char *)QBox_Json_GetString(root,"bucket",NULL);
	tpl->Dissolve = (char *)QBox_Json_GetString(root,"dissolve",NULL);
	tpl->Gravity = (char *)QBox_Json_GetString(root,"gravity",NULL);
	tpl->Dx = QBox_Json_GetInt64(root,"dx",0);
	tpl->Dy = QBox_Json_GetInt64(root,"dy",0);

	return err;
}


QBox_Error QBox_WM_Set( QBox_Client* self, QBox_WM_Template *tpl, char *customer )
{
	QBox_Error err;
	cJSON* root;
	char body[65535];

	char* url = QBox_String_Concat(QBOX_EU_HOST, "/wmset", NULL);

	snprintf(body, 65535, 
		"customer=%s&font=%s&pointsize=%d&fill=%s&text=%s&bucket=%s&dissolve=%s&gravity=%s&dx=%d&dy=%d", 
		customer ? customer : "",
		tpl->Font ? tpl->Font : "",
		tpl->PointSize,
		tpl->Fill ? tpl->Fill : "",
		tpl->Text ? tpl->Text : "",
		tpl->Bucket ? tpl->Bucket : "",
		tpl->Dissolve ? tpl->Dissolve : "",
		tpl->Gravity ? tpl->Gravity : "",
		tpl->Dx,tpl->Dy );

	err = QBox_Client_CallWithForm(self, &root, url, body, strlen(body));
	free(url);

	return err;
}