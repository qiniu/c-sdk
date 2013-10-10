/*******************************************************************************
 *  @file      encoding_in_windows.h 2013\10\10 16:36:24 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com>(中文编码测试)
 *             Windows 本地代码页(char*)  <=> Unicode-utf8（char*)
 *             Windiws 本地代码页(char*)  <=> Unicode-utf16(wchar_t*)
 *             Unicode-utf16(wchar_t*)   <=> Unicode-utf8（char*)
 ******************************************************************************/

#ifndef ENCODING_IN_WINDOWS_96BEAFAE_C869_42F6_8B02_128E51A7A7F6_H__
#define ENCODING_IN_WINDOWS_96BEAFAE_C869_42F6_8B02_128E51A7A7F6_H__


#ifdef __cplusplus
extern "C"
{
#endif/*__cplusplus*/

#if defined(_WIN32) || defined(_WIN64)

#include <stddef.h>

/******************************************************************************/

/** @name 字符串编码格式转换，仅仅用于windows 平台*/
/* @{*/

/**
 * 在windows 上 wchar_t(Unicode)到 utf8char（utf-8)的转换
 *
 * @param   const wchar_t * src 要转换的字符串
 * @param   size_t          len 字符串所占字符的长度，不包含字符串结束字符L'\0'
 * @return  char*               如果转换成功 ： 返回 转换后的utf-8字符串指针，此字符串以'\0'结尾
 *                              此资源指针需要调用函数<code>Qiniu_Free(void* addr)</code>释放
 *                              如果转换不成功 ： 返回 空指针 0
 */
char*    Qiniu_UTF16_To_UTF8(const wchar_t* src, size_t len);

/**
 * 在Windows上从char(utf-8)到wchar_t(Unicode)的转换
 *
 * @param   const char * src    要转换的字符串
 * @param   size_t       len    字符串所占字符的长度，其不包含字符串结束字符'\0'
 * @return  wchar_t*            如果转换成功 ： 返回 转换后的Unicode字符串指针，此字符串以L'\0'结尾
 *                              此资源指针需要调用函数<code>Qiniu_Free(void* addr)</code>释放
 *                              如果转换不成功 ： 返回 空指针 0
 */
wchar_t* Qiniu_UTF8_To_UTF16(const char* src,  size_t len);


/**
 * 在windows上转换wchar_t(Unicode)到 char(本地代码页)的转换。
 *
 * @param   const wchar_t * src 要转换的字符串
 * @param   size_t          len 字符串所占字符的长度，其不包含字符串结束字符L'\0'
 * @return  char*               如果转换成功 ： 返回 转换后的本地代码页字符串指针，此字符串以'\0'结尾
 *                              此资源指针需要调用函数<code>Qiniu_Free(void* addr)</code>释放
 *                              如果转换不成功 ： 返回 空指针 0
 */
char*    Qiniu_UTF16_To_LocalCodePage(const wchar_t* src, size_t len);

/**
 * 在Windows上从char(本地代码页)到wchar_t(Unicode)的转换
 *
 * @param   const char * src  要转换的字符串
 * @param   size_t       len  字符串所占字符的长度，其不包含字符串结束字符'\0'
 * @return  wchar_t*          如果转换成功 ： 返回 转换后的Unicode字符串指针，此字符串以L'\0'结尾
 *                            此资源指针需要调用函数<code>Qiniu_Free(void* addr)</code>释放
 *                            如果转换不成功 ： 返回 空指针 0
 */
wchar_t* Qiniu_LocalCodePage_To_UTF16(const char* src, size_t len);

/**
 * 在Windows上从char(本地代码页)到char(utf-8)的转换
 *
 * @param   const char * src 要转换的字符串
 * @param   size_t       len 字符串所占字符的长度，其不包含字符串结束字符'\0'
 * @return  char*            如果转换成功 ： 返回 转换后utf-8字符串指针，此字符串以'\0'结尾
 *                           此资源指针需要调用函数<code>Qiniu_Free(void* addr)</code>释放
 *                           如果转换不成功 ： 返回 空指针 0
 */
char* Qiniu_LocalCodePage_To_UTF8(const char* src, size_t len);

/**
 *  在Windows上从utf8char(utf-8)到char(本地代码页)的转换
 *
 * @param   const char * src 要转换的字符串
 * @param   size_t       len 字符串所占字符的长度，其不包含字符串结束字符'\0'
 * @return  char*            如果转换成功 ： 返回 转换后的本地代码页字符串指针，此字符串以'\0'结尾
 *                           此资源指针需要调用函数<code>Qiniu_Free(void* addr)</code>释放
 *                           如果转换不成功 ： 返回 空指针 0
 * @exception 
 */
char* Qiniu_UTF8_To_LocalCodePage(const char* src, size_t len);

/* @}*/

#endif /*defined(_WIN32) || defined(_WIN64)*/

#ifdef __cplusplus
}
#endif /*__cplusplus*/

/******************************************************************************/
#endif// ENCODING_IN_WINDOWS_96BEAFAE_C869_42F6_8B02_128E51A7A7F6_H__
