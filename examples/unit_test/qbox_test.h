/*
 ============================================================================
 Name        : qbox_test.h
 Author      : Jiang Wen Long
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */

#ifndef QBOX_TEST_H
#define QBOX_TEST_H

#include "../../qbox/oauth2.h"

static const char* TABLE = "c-test";

void QBT_Do(QBox_Client* client);

#define QBT_Fatalf						\
	printf("\t[FATAL]%s:%d => ", __FILE__, __LINE__);	\
	return _QBT_Printf

#define QBT_Infof						\
	printf("\t[INFO]%s:%d => ", __FILE__, __LINE__);	\
	_QBT_Printf

int _QBT_Printf(const char* fmt, ...);

#define QBT_CheckErr(err)		\
	do {				\
		if (err.code != 200) {	\
			QBT_Fatalf("code: %d, msg: %s\n", err.code, err.message);	\
		}			\
	}while (0)

#endif
