 /*
 ============================================================================
 Name        : suites_base.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/auth_policy.h"
#include "../qbox/rs.h"
#include "test.h"

QBox_Error err;
QBox_Client client;

void test_QBox_MakeUpToken(){
    char* uptoken;
    QBox_AuthPolicy auth;
    auth.scope="test";
    auth.callbackUrl="test";
    auth.returnUrl="test";
    auth.expires=1800;
    uptoken = NULL;
	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
		printf("Cannot generate UpToken!\n");
		return;
	}

    auth.scope=NULL;
    auth.callbackUrl=NULL;
    auth.returnUrl=NULL;
    auth.expires=0;
    uptoken = NULL;
	uptoken = QBox_MakeUpToken(&auth);
	if (uptoken == NULL) {
		printf("Cannot generate UpToken!\n");
		return;
	}
}

/**//*---- test suites ------------------*/
int suite_init_auth_policy(void)
{
	QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_auth_policy(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(auth_policy)
QBOX_TEST(test_QBox_MakeUpToken)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(auth_policy,suite_init_auth_policy,suite_clean_auth_policy)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/
void AddTestsAuthPolicy(void)
{
        QBOX_TEST_REGISTE(auth_policy)
}

