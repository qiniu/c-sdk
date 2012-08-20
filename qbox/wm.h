#ifndef QBOX_WM_H
#define QBOX_WM_H

#include "oauth2.h"
#include "base.h"

typedef struct _QBox_WM_Template
{
	char*	Font;
	int		PointSize;
	char*	Fill;
	char*	Text;
	char*	Bucket;
	char*	Dissolve;
	char*	Gravity;
	int		Dx;
	int 	Dy;
} QBox_WM_Template;


QBox_Error QBox_WM_Get( QBox_Client* self, char *customer, QBox_WM_Template *tpl );
QBox_Error QBox_WM_Set( QBox_Client* self, QBox_WM_Template *tpl, char *customer );

#endif