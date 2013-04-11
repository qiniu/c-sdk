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
#include "../../c-sdk-2.2.0/qbox/image.h"
#include "../../c-sdk-2.2.0/qbox/rs.h"
#include "../../c-sdk-2.2.0/qbox/base.h"
#include "c_unit_test_main.h"


QBox_Error err;
QBox_Client client;

#define MY_TRUE 0
#define MY_FALSE -1

//extern char* _dirtycat(char* dst, const char* src);
void test_QBox_IMG_Info(){
    QBox_IMG_InfoRet* ret=malloc(sizeof(QBox_IMG_InfoRet));
    const char* imgURL="http://qiniuphotos.dn.qbox.me/gogopher.jpg";
    err=QBox_IMG_Info(&client,ret,imgURL);
    CU_ASSERT_EQUAL(err.code,200);
    //test branch:err!=200.
    imgURL="http://upload.mjdo.com/lady/20110621/12/201121733.jpg";
    err=QBox_IMG_Info(&client,ret,imgURL);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    //printf("\n%d\n%s\n",err.code,err.message);
    free(ret);
}

void test_QBox_IMG_Exif(){
    const char* imgURL="http://qiniuphotos.dn.qbox.me/gogopher.jpg";
    QBox_IMG_ExifRet* ret=malloc(sizeof(QBox_IMG_ExifRet));


	err = QBox_IMG_Exif(&client, ret, imgURL);
	CU_ASSERT_EQUAL(err.code,200);
    /*
    printf("\n");
	QBox_Uint32 index;
	for (index = 0; index < ret->size; index++) {
		printf("name: %-25s val: %-20s type: %d\n",
				ret->info[index].name,
				ret->info[index].val,
				(int)ret->info[index].type);
	}
    */

    imgURL="http://qiniuphotos.dn.qbox.me/gogopherxx.jpg";
	err = QBox_IMG_Exif(&client, ret, imgURL);
	CU_ASSERT_NOT_EQUAL(err.code,200);

	QBox_IMG_ExifRet_Release(*ret);
}

void test_QBox_IMG_InitViewOpts(){
    QBox_IMG_ViewOpts* opts=malloc(sizeof(QBox_IMG_ViewOpts));
    QBox_IMG_InitViewOpts(opts);
    free(opts);
}


void test_QBox_IMG_ViewURL(){
    const char* imgURL="http://qiniuphotos.dn.qbox.me/gogopher.jpg";
	char* url = NULL;
	char* urlCompare = malloc(sizeof(char)*128);
	QBox_IMG_ViewOpts opts;
	QBox_IMG_InitViewOpts(&opts);

	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/0");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);

	opts.mode = 1;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);

	opts.width = 100;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);


	opts.height = 200;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);

	opts.quality = 50;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);

	opts.format = "png";
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/format/png");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);


	opts.sharpen = 60;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/format/png/sharpen/60");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);

	opts.watermark = 1;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	//printf("url:%s\n", url);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/format/png/sharpen/60/watermark/1");
	CU_ASSERT_EQUAL(strcmp(url,urlCompare),0);
	free(url);

    //test branch: src2nd[0] == '\0' in function _dirtycat2
	opts.format = "\0";
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/format/\0");
	CU_ASSERT_NOT_EQUAL(strcmp(url,urlCompare),0);
	free(url);

	free(urlCompare);
}

void test_QBox_IMG_MogrifyURL(){
    const char* imgURL="http://qiniuphotos.dn.qbox.me/gogopher.jpg";
	char* url = NULL;
	char* urlCompare = malloc(sizeof(char)*256);
	QBox_IMG_MogrOpts opts;
	QBox_IMG_InitMogrOpts(&opts);

	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	free(url);

	opts.auto_orient = 1;
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/auto-orient");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	free(url);

	opts.quality = "60";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/quality/60/auto-orient");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	free(url);

	opts.thumbnail = "50%";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/quality/60/auto-orient");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	free(url);

	opts.gravity = "North";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/quality/60/auto-orient");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	free(url);

	opts.crop = "!300x400a10a10";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/auto-orient");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	free(url);

	opts.rotate = "45";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/rotate/45/auto-orient");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	free(url);


	opts.format = "png";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/rotate/45/format/png/auto-orient");
	CU_ASSERT_EQUAL(strcmp(url, urlCompare),0);
	//printf("\n%s\n", url);
	free(url);

	free(urlCompare);
}

void test_QBox_IMG_SaveAs(){
	QBox_RS_GetRet getRet;
	QBox_RS_PutRet putRet;
	QBox_IMG_SaveAsRet ret;
	QBox_IMG_MogrOpts opts;
	QBox_IMG_InitMogrOpts(&opts);
    const char* tableName="c_testImage_table_0";
	char* imgURL = NULL;
    err = QBox_RS_Create(&client,tableName);
	char* file="/home/wsy/文档/SDKUnitTest/src/gogopher.jpg";
	err = QBox_RS_PutFile(&client, &putRet, tableName, "gogopher.jpg", "image/jpeg", file, "");
	CU_ASSERT_EQUAL(err.code,200);

	err = QBox_RS_Get(&client, &getRet, tableName, "gogopher.jpg", NULL);
	CU_ASSERT_EQUAL(err.code,200);

	imgURL = (char*)malloc(strlen(getRet.url)*sizeof(char));
	strcpy(imgURL, getRet.url);

	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save1");
	CU_ASSERT_EQUAL(err.code,200);

	opts.auto_orient = 1;
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save2");
	CU_ASSERT_EQUAL(err.code,200);

	opts.quality = "60";
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save3");
	CU_ASSERT_EQUAL(err.code,200);

	opts.thumbnail = "!120x120r";
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save4");
	CU_ASSERT_EQUAL(err.code,200);

	opts.gravity = "North";
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save5");
	CU_ASSERT_EQUAL(err.code,200);

	opts.crop = "!300x400a10a10";
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save6");
	CU_ASSERT_EQUAL(err.code,200);

	opts.rotate = "45";
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save7");
	CU_ASSERT_EQUAL(err.code,200);

	opts.format = "png";
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "save8");
	CU_ASSERT_EQUAL(err.code,200);


    QBOX_ACCESS_KEY = "test Err!=200";
	err = QBox_IMG_SaveAs(&client, &ret, &opts, imgURL, tableName, "saveErr");
	CU_ASSERT_NOT_EQUAL(err.code,200);
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";

	free(imgURL);
}

///in image.h, the interface is non-existent
/*
void test__dirtycat(){
    char *dst=malloc(sizeof(char)*128);
    strcpy(dst,"test");
    //_dirtycat(dst,NULL);
    CU_ASSERT_EQUAL(strcmp(dst,"test"),0);
}
*/

CU_TestInfo testcases_image[] = {
        {"Testing QBox_IMG_Info:", test_QBox_IMG_Info},
        {"Testing QBox_IMG_Exif:", test_QBox_IMG_Exif},
        {"Testing QBox_IMG_InitViewOpts:", test_QBox_IMG_InitViewOpts},
        {"Testing QBox_IMG_ViewURL:", test_QBox_IMG_ViewURL},
        {"Testing QBox_IMG_MogrifyURL:", test_QBox_IMG_MogrifyURL},
        {"Testing QBox_IMG_SaveAs:", test_QBox_IMG_SaveAs},
        //{"Testing _dirtycat:", test__dirtycat},
        CU_TEST_INFO_NULL
};


/**//*---- test suites ------------------*/
int suite_image_init(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_image_clean(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();
    return 0;
}

CU_SuiteInfo suites_image[] = {
        {"Testing the qbox.image:", suite_image_init, suite_image_clean, testcases_image},
        CU_SUITE_INFO_NULL
};


/**//*---- setting enviroment -----------*/

void AddTestsImage(void)
{
        assert(NULL != CU_get_registry());
        assert(!CU_is_test_running());
        /**//* shortcut regitry */

        if(CUE_SUCCESS != CU_register_suites(suites_image)){
                fprintf(stderr, "Register suites failed - %s ", CU_get_error_msg());
                exit(EXIT_FAILURE);
        }
}
