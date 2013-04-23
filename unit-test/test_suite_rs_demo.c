/*
 ============================================================================
 Name        : qbox_test.h
 Author      : Wu Shi Yu
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/TestDB.h>
#include "../qbox/base.h"
#include "../qbox/rs.h"
#include "../qbox/rscli.h"
#include "../qbox/oauth2_passwd.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

void test_by_rs_demo(){
	QBox_Token* token;
	QBox_RS_PutAuthRet putAuthRet;
	QBox_RS_PutRet putRet;
	QBox_RS_GetRet getRet;
	QBox_RS_StatRet statRet;
	char* hash;

	const char* tableName="test_Rs_demo_table_0";


	//printf("\n    QBox_Token_ExchangeByPassword\n");
	err = QBox_Token_ExchangeByPassword(&token, "test@qbox.net", "test");
	CU_ASSERT_EQUAL(err.code,200);

	QBox_Client_InitByPassword(&client, token, 1024);
	QBox_Token_Release(token);
	QBox_RS_Delete(&client, "Bucket", "rs_demo.c");

	//printf("    QBox_RS_PutFile\n");
    err = QBox_RS_Create(&client,tableName);
    CU_ASSERT_EQUAL(err.code,200);

	err = QBox_RS_PutFile(&client, &putRet, tableName, "rs/demo.c", "application/octet-stream", __FILE__, "");
	CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_Get\n");
	err = QBox_RS_Get(&client, &getRet, tableName, "rs/demo.c", NULL);
	CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_PutAuth\n");
	err = QBox_RS_PutAuth(&client, &putAuthRet);
	CU_ASSERT_EQUAL(err.code,200);

    //*
    char* pStream = NULL;
    int bytes=128;
    pStream = malloc(bytes);
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rs_demo4.c","application/octet-stream",pStream,bytes,"","key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rs_demo4.c","application/octet-stream",pStream,bytes,"test","key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rs_demo4.c","application/octet-stream",pStream,bytes,NULL,"key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
	err=QBox_RSCli_PutStream(NULL,putAuthRet.url,tableName,"rs_demo4.c",NULL,pStream,bytes,NULL,"key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    free(pStream);
    //*/

	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, tableName, "rs_demo.c", "application/octet-stream", __FILE__, "", "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RSCli_PutFile\n");
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, tableName, "rs_demo.c", "application/octet-stream", __FILE__, "", "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType=NULL customMeta=NULL
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, tableName, "rs_demo1.c", NULL, __FILE__, NULL, "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType=NULL customMeta!=NULL & *customMeta!='\0'
	err = QBox_RSCli_PutFile(NULL, putAuthRet.url, tableName, "rs_demo2.c", NULL, __FILE__, "test", "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:mimeType=NULL customMeta!=NULL & *customMeta!='\0'
	err = QBox_RSCli_PutFile(NULL, NULL, tableName, "rs_demo2.c", NULL, __FILE__, "test", "key=rs_demo.c");
    CU_ASSERT_NOT_EQUAL(err.code,200);
    //test branch: resp!=NULL
    QBox_Buffer* buffer=malloc(sizeof(QBox_Buffer));
    QBox_Buffer_Init(buffer,1024);
	err = QBox_RSCli_PutFile(buffer, putAuthRet.url, tableName, "rs_demo2.c", NULL, __FILE__, "test", "key=rs_demo.c");
    CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_Get\n");
	err = QBox_RS_Get(&client, &getRet, tableName, "rs_demo.c", NULL);
	CU_ASSERT_EQUAL(err.code,200);
	hash = strdup(getRet.hash);

	//printf("    QBox_RS_GetIfNotModified: %s\n", hash);
	err = QBox_RS_GetIfNotModified(&client, &getRet, tableName, "rs_demo.c", NULL, hash);
	free(hash);
	CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_Stat\n");
	err = QBox_RS_Stat(&client, &statRet, tableName, "rs_demo.c");
	CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_Publish\n");
	err = QBox_RS_Publish(&client, tableName, "iovip.qbox.me/Bucket");
	CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_Unpublish\n");
	err = QBox_RS_Unpublish(&client, "iovip.qbox.me/Bucket");
	CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_Delete\n");
	err = QBox_RS_Delete(&client, tableName, "rs_demo.c");
	CU_ASSERT_EQUAL(err.code,200);
	err = QBox_RS_Delete(&client, tableName, "rs_demo1.c");
	CU_ASSERT_EQUAL(err.code,200);
	err = QBox_RS_Delete(&client, tableName, "rs_demo2.c");
	CU_ASSERT_EQUAL(err.code,200);

	//printf("    QBox_RS_Drop\n");
	err = QBox_RS_Drop(&client, tableName);
	CU_ASSERT_EQUAL(err.code,200);

lzDone:
	return ;
}




/**//*---- test suites ------------------*/
int suite_init_rs_demo(void)
{
	QBox_Zero(client);
	QBox_Global_Init(-1);

	return 0;
}

int suite_clean_rs_demo(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(rs_demo)
QBOX_TEST(test_by_rs_demo)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(rs_demo,suite_init_rs_demo,suite_clean_rs_demo)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsRsDemo(void)
{
        QBOX_TEST_REGISTE(oauth2_passwd)
}

