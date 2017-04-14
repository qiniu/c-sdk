/*
 ============================================================================
 Name        : tm.c
 Author      : Qiniu.com
 Copyright   : 2016(c) Shanghai Qiniu Information Technologies Co., Ltd.
 Description :
 ============================================================================
 */

#include "tm.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32

#include "../windows/emu_posix.h" // for type Qiniu_Posix_GetTimeOfDay

QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Tm_LocalTime(void)
{
	return Qiniu_Posix_GetTimeOfDay();
} // Qiniu

#else

#include <sys/time.h>

QINIU_DLLAPI extern Qiniu_Uint64 Qiniu_Tm_LocalTime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec;
} // Qiniu_Tm_LocalTime

#endif

#ifdef __cplusplus
}
#endif

