/******************************************************************************* 
 *  @file      encoding_in_windows.c 2013\10\10 16:33:39 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 ******************************************************************************/
#include "encoding_in_windows.h"

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>
#include <stdlib.h>
/******************************************************************************/

char* Qiniu_UTF16_To_UTF8( const wchar_t* src, size_t len )
{
    int cch2        = WideCharToMultiByte(CP_UTF8, 0, src, len, 0, 0, 0, 0);
    char* pReturn   = (char*)malloc(sizeof(char)*(cch2 + 1));
    int returnValue = WideCharToMultiByte(CP_UTF8, 0, src, len, pReturn, cch2 + 1, 0, 0);

    if (returnValue == 0)
    {
        free(pReturn);
        pReturn = 0;
    }
    else
    {
        pReturn[cch2] = 0;
    }

    return pReturn;
}


wchar_t* Qiniu_UTF8_To_UTF16( const char* src, size_t len )
{
    int cch2 = MultiByteToWideChar(CP_UTF8, 0, src, len, NULL, 0);
    wchar_t* pReturn = (wchar_t*)malloc(sizeof(wchar_t)*(cch2 + 1));
    int returnValue = MultiByteToWideChar(CP_UTF8, 0, src, len, pReturn, cch2);

    if (returnValue == 0)
    {
        free(pReturn);
        pReturn = 0;
    }
    else
    {
        pReturn[cch2] = 0;
    }
    return pReturn;
}

char* Qiniu_UTF16_To_LocalCodePage( const wchar_t* src, size_t len )
{
    int cch2         = WideCharToMultiByte(CP_ACP, 0, src, len, 0, 0, 0, 0);
    char* pReturn    = (char*)malloc(sizeof(char)*(cch2 + 1));
    int returnValue = WideCharToMultiByte(CP_ACP, 0, src, len, pReturn, cch2 + 1, 0, 0);
    if (returnValue == 0)
    {
        free(pReturn);
        pReturn = 0;
    }
    else
    {
        pReturn[cch2] = 0;
    }
    return pReturn;
}


wchar_t* Qiniu_LocalCodePage_To_UTF16( const char* src, size_t len )
{
    int cch2         = MultiByteToWideChar(CP_ACP, 0, src, len, NULL, 0);
    wchar_t* pReturn = (wchar_t*)malloc(sizeof(wchar_t)*(cch2 + 1));
    int returnValue  = MultiByteToWideChar(CP_ACP, 0, src, len, pReturn, cch2);
    if (returnValue == 0)
    {
        free(pReturn);
        pReturn = 0;
    }    
    else
    {
        pReturn[cch2] = 0;
    }
    return pReturn;
}


char* Qiniu_LocalCodePage_To_UTF8( const char* src, size_t len )
{

    char* pReturn = 0;
    wchar_t* pwtem = Qiniu_LocalCodePage_To_UTF16(src, len);
    if (0 != pwtem)
    {
        pReturn = Qiniu_UTF16_To_UTF8(pwtem, wcslen(pwtem));
        free(pwtem);
    }

    return pReturn;
}


char* Qiniu_UTF8_To_LocalCodePage( const char* src, size_t len )
{
    char* pReturn  = 0;
    wchar_t* pwtem = Qiniu_UTF8_To_UTF16(src, len);
    if (0 != pwtem)
    {
        pReturn = Qiniu_UTF16_To_LocalCodePage(pwtem, wcslen(pwtem));
        free(pwtem);
    }

    return pReturn;
}

#endif/*defined(_WIN32) || defined(_WIN64)*/
/******************************************************************************/

