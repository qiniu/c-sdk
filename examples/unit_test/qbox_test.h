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

void QBT_Do(QBox_Client* client);

#define QBT_Fatalfln						\
	printf("\t[ERROR]%s:%d => ", __FILE__, __LINE__);	\
	return _QBT_Errorfln
int _QBT_Errorfln(const char* fmt, ...);

#endif
