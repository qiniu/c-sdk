/*
 ============================================================================
 Name        : test.c
 Author      : Qiniu Developers
 Version     : 1.0.0.0
 Copyright   : 2012(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "CUnit/Basic.h"
#include "test.h"


void AddTests(void);

int main(int argc, char* argv[])
{
    CU_BasicRunMode mode = CU_BRM_VERBOSE;
    CU_ErrorAction error_action = CUEA_IGNORE;
    int i;

    myMode=0;

    setvbuf(stdout, NULL, _IONBF, 0);

    for (i=1 ; i<argc ; i++) {
        if (!strcmp("-i", argv[i])) {
            error_action = CUEA_IGNORE;
        }
        else if (!strcmp("-f", argv[i])) {
            error_action = CUEA_FAIL;
        }
        else if (!strcmp("-A", argv[i])) {
            error_action = CUEA_ABORT;
        }
        else if (!strcmp("-s", argv[i])) {
            mode = CU_BRM_SILENT;
        }
        else if (!strcmp("-n", argv[i])) {
            mode = CU_BRM_NORMAL;
        }
        else if (!strcmp("-v", argv[i])) {
            mode = CU_BRM_VERBOSE;
        }
        else if (!strcmp("-b", argv[i])) {
            myMode=myMode|ADD_BAD_TEST;
        }
        else if (!strcmp("-x", argv[i])) {
            myMode=myMode|GENERATE_XML;
        }
        else if (!strcmp("-e", argv[i])) {
            return 0;
        }
        else {
            printf("\nUsage: BasicTest [options]\n\n"
                   "Options: -i     ignore framework errors [default].\n"
                   "         -f     fail on framework error.\n"
                   "         -A     abort on framework error.\n\n"
                   "         -s     silent mode - no output to screen.\n"
                   "         -n     normal mode - standard output to screen.\n"
                   "         -v     verbose mode - max output to screen [default].\n\n"
                   "         -b     add some bad test which spend much time.\n"
                   "         -x     xml mode - no output to screen.generate XML.\n\n"
                   "         -e     print expected test results and exit.\n"
                   "         -h     print this message and exit.\n\n");
            return 0;
        }
    }

    if (CU_initialize_registry()) {
        printf("\nInitialization of Test Registry failed.");
    }
    else {
        /*/

        AddTestsRS();//* //1
        AddTestsRsCli();//*  //4
        AddTestsImage();//* //3
        AddTestsUp(myMode);//* //5
        AddTestsRsUp();//* //6
        AddTestsOauth2();//* //7
        AddTestsAuthPolicy();//* //8
        //AddTestsIo();//*/ //9?
        AddTestsBase();//*  //2
        //AddTestsOauth2Passwd();//*/
        //AddTestsUpDemoResumable(myMode);//*

        //*/
        if((myMode&GENERATE_XML)!=0){
            //***使用自动产生XML文件的模式********
            CU_set_output_filename("c-sdk-unit-test");
            CU_list_tests_to_file();
            CU_automated_run_tests();
            //***********************************/
        }
        else{
            CU_basic_set_mode(mode);
            CU_set_error_action(error_action);
            printf("\nTests completed with return value %d.\n", CU_basic_run_tests());
        }
        CU_cleanup_registry();
    }

    return 0;
}
