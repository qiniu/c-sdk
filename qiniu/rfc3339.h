#ifndef QINIU_RFC3339_H
#define QINIU_RFC3339_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DAY_IN_SECS 86400
#define HOUR_IN_SECS 3600
#define MINUTE_IN_SECS 60
#define HOUR_IN_MINS 60

#if defined(_WIN32)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct _Qiniu_Date
    {
        unsigned int year;
        unsigned int month;
        unsigned int day;
        char ok;
    } Qiniu_Date;

    typedef struct _Qiniu_Time
    {
        unsigned int hour;
        unsigned int minute;
        unsigned int second;
        unsigned int fraction;
        int offset; // UTC offset in minutes
        char ok;
    } Qiniu_Time;

    typedef struct _Qiniu_DateTime
    {
        Qiniu_Date date;
        Qiniu_Time time;
        char ok;
    } Qiniu_DateTime;

#if defined(_WIN32)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif // QINIU_RFC3339_H
