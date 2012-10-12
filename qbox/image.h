/*
 ============================================================================
 Name        : image.h
 Author      : Jiang WenLong
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description : 
 ============================================================================
 */

#ifndef QBOX_IMAGE_H
#define QBOX_IMAGE_H

/*============================================================================*/
/* func QBox_IMG_Info */

#include "oauth2.h"

typedef struct _QBox_IMG_InfoRet {
	const char* format;	// "png", "jpeg", "gif", "bmp", etc.
	const char* colorModel;	// "palette16", "ycbcr", etc.
	QBox_Int64 width;
	QBox_Int64 height;
} QBox_IMG_InfoRet;

QBox_Error QBox_IMG_Info(
		QBox_Client* self, QBox_IMG_InfoRet* ret, const char* url);

#endif
