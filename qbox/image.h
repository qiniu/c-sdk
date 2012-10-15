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

/*============================================================================*/
/* func QBox_IMG_MogrifyUrl */

typedef struct _QBox_IMG_MogrOpts {
	const char* thumbnail;
	const char* gravity;
	const char* crop;
	const char* quality;
	const char* rotate;
	const char* format;
	QBox_Bool auto_orient;
} QBox_IMG_MogrOpts;

void QBox_IMG_InitMogrOpts(QBox_IMG_MogrOpts* opts);

// remember to free the returned pointer when not needed anymore.
char* QBox_IMG_MogrifyURL(QBox_IMG_MogrOpts* opts, const char* url);

/*============================================================================*/
/* func QBox_IMG_SaveAs */

typedef struct _QBox_IMG_SaveAsRet {
	const char* hash;
} QBox_IMG_SaveAsRet;

QBox_Error QBox_IMG_SaveAs(QBox_Client* self, QBox_IMG_SaveAsRet* ret, 
		QBox_IMG_MogrOpts* opts, const char* url,
		const char* tableName, const char* key);

#endif
