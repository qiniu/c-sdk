/*
 ============================================================================
 Name        : suites_image.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../CUnit/CUnit/Headers/CUnit.h"
#include "../CUnit/CUnit/Headers/Automated.h"
#include "../CUnit/CUnit/Headers/TestDB.h"
#include "../qbox/image.h"
#include "../qbox/rs.h"
#include "../qbox/base.h"
#include "test.h"


QBox_Error err;
QBox_Client client;

#define MY_TRUE 0
#define MY_FALSE -1
//#define PICTURE "/home/wsy/文档/c-sdk/unit-test/gogopher.jpg"
#define PICTURE "gogopher.jpg"

//extern char* _dirtycat(char* dst, const char* src);
void test_QBox_IMG_Info(){
    QBox_IMG_InfoRet* ret=malloc(sizeof(QBox_IMG_InfoRet));
    const char* imgURL="http://qiniuphotos.dn.qbox.me/gogopher.jpg";
    err=QBox_IMG_Info(&client,ret,imgURL);
    CU_ASSERT_EQUAL(err.code,200);
    CU_ASSERT_STRING_EQUAL(ret->format,"jpeg");
    CU_ASSERT_STRING_EQUAL(ret->colorModel,"ycbcr");
    CU_ASSERT_EQUAL(ret->width,640);
    CU_ASSERT_EQUAL(ret->height,427);
    //test branch:err!=200.
    imgURL="http://upload.mjdo.com/lady/20110621/12/201121733.jpg";
    err=QBox_IMG_Info(&client,ret,imgURL);
    CU_ASSERT_NOT_EQUAL(err.code,200);
    CU_ASSERT_EQUAL(err.code,405);
    free(ret);
}

void test_QBox_IMG_Exif(){
    char* imgURL=malloc(128);
    QBox_IMG_ExifRet* ret=malloc(sizeof(QBox_IMG_ExifRet));
    imgURL="http://qiniuphotos.dn.qbox.me/gogopher.jpg";
	err = QBox_IMG_Exif(&client, ret, imgURL);
	CU_ASSERT_EQUAL(err.code,200);
	/*
	for (index = 0; index < ret->size; index++) {
		printf("name: %-25s val: %-20s type: %d\n",
				ret->info[index].name,
				ret->info[index].val,
				(int)ret->info[index].type);
	}*/
	QBox_IMG_ExifRet_Release(*ret);

    imgURL="http://qiniuphotos.dn.qbox.me/gogopherxx.jpg";
	err = QBox_IMG_Exif(&client, ret, imgURL);
	CU_ASSERT_NOT_EQUAL(err.code,200);
}

void test_QBox_IMG_InitViewOpts(){
    QBox_IMG_ViewOpts* opts=malloc(sizeof(QBox_IMG_ViewOpts));
    QBox_IMG_InitViewOpts(opts);
    CU_ASSERT_EQUAL(opts->width,-1);
    CU_ASSERT_EQUAL(opts->height,-1);
    CU_ASSERT_EQUAL(opts->quality,-1);
    CU_ASSERT_EQUAL(opts->sharpen,-1);
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
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.mode = 1;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.width = 100;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);


	opts.height = 200;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.quality = 50;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.format = "png";
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/format/png");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);


	opts.sharpen = 60;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/format/png/sharpen/60");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.watermark = 1;
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/format/png/sharpen/60/watermark/1");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

    //test branch: src2nd[0] == '\0' in function _dirtycat2
	opts.format = "\0";
	url = QBox_IMG_ViewURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageView/1/w/100/h/200/q/50/sharpen/60/watermark/1");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);
	free(urlCompare);
}

void test_QBox_IMG_InitMogrOpts(){
    QBox_IMG_MogrOpts* opts=malloc(sizeof(QBox_IMG_MogrOpts));
    QBox_IMG_InitMogrOpts(opts);
	CU_ASSERT_EQUAL(opts->thumbnail,0);
	CU_ASSERT_EQUAL(opts->gravity,0);
	CU_ASSERT_EQUAL(opts->crop,0);
	CU_ASSERT_EQUAL(opts->quality,0);
	CU_ASSERT_EQUAL(opts->rotate,0);
	CU_ASSERT_EQUAL(opts->format,0);
	CU_ASSERT_EQUAL(opts->auto_orient,0);
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
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.auto_orient = 1;
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/auto-orient");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.quality = "60";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/quality/60/auto-orient");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.thumbnail = "50%";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/quality/60/auto-orient");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.gravity = "North";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/quality/60/auto-orient");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.crop = "!300x400a10a10";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/auto-orient");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);

	opts.rotate = "45";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/rotate/45/auto-orient");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
	free(url);


	opts.format = "png";
	url = QBox_IMG_MogrifyURL(&opts, imgURL);
	strcpy(urlCompare,imgURL);
	strcat(urlCompare,"?imageMogr/thumbnail/50%/gravity/North/crop/!300x400a10a10/quality/60/rotate/45/format/png/auto-orient");
	CU_ASSERT_STRING_EQUAL(url,urlCompare);
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
	char* file=PICTURE;
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

    //test Error
    //test err401
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


/**//*---- test suites ------------------*/
int suite_init_image(void)
{
    QBOX_ACCESS_KEY = "cg5Kj6RC5KhDStGMY-nMzDGEMkW-QcneEqjgP04Z";
	QBOX_SECRET_KEY = "yg6Q1sWGYBpNH8pfyZ7kyBcCZORn60p_YFdHr7Ze";

	QBox_Zero(client);
	QBox_Global_Init(-1);

	QBox_Client_Init(&client, 1024);

	return 0;
}

int suite_clean_image(void)
{
	QBox_Client_Cleanup(&client);
	QBox_Global_Cleanup();

    return 0;
}

QBOX_TESTS_BEGIN(image)
QBOX_TEST(test_QBox_IMG_Info)
QBOX_TEST(test_QBox_IMG_Exif)
QBOX_TEST(test_QBox_IMG_InitViewOpts)
QBOX_TEST(test_QBox_IMG_ViewURL)
QBOX_TEST(test_QBox_IMG_InitMogrOpts)
QBOX_TEST(test_QBox_IMG_MogrifyURL)
QBOX_TEST(test_QBox_IMG_SaveAs)
QBOX_TESTS_END()

QBOX_SUITES_BEGIN()
QBOX_SUITE_EX(image,suite_init_image,suite_clean_image)
QBOX_SUITES_END()


/**//*---- setting enviroment -----------*/

void AddTestsImage(void)
{
        QBOX_TEST_REGISTE(image)
}
