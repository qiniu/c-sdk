#include "rfc3339.h"

static void _Strip_Spaces(char *source)
{
    char *i = source;

    while (*source != 0)
    {
        *i = *source++;
        if (*i != ' ')
            i++;
    }

    *i = 0;
}

static void _Qiniu_Parse_Date(char *date_string, Qiniu_Date *d)
{
    char *const tokens = strdup(date_string);
    _Strip_Spaces(tokens);
    d->ok = 0;

    if (strlen(tokens) < 10)
    {
        return;
    }

    int status = sscanf(
        tokens, "%04d-%02d-%02d", &(d->year), &(d->month), &(d->day));
    free((char *)tokens);

    if (status != 3)
    {
        return;
    }
    if (d->year < 1 || d->year > 9999)
    {
        return;
    }
    if (d->month < 1 || d->month > 12)
    {
        return;
    }
    if (d->day < 1 || d->day > 31)
    {
        return;
    }

    const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (d->month == 2)
    {
        int leap = (d->year % 4 == 0 && d->year % 100 != 0) || (d->year % 400 == 0);
        if (d->day > (days_in_month[2 - 1] + leap))
        {
            return;
        }
    }
    else if (d->day > days_in_month[d->month - 1])
    {
        return;
    }

    d->ok = 1;
}

static void _Qiniu_Parse_Time(char *time_string, Qiniu_Time *t)
{
    char *tokens = strdup(time_string);
    char *token_ptr = tokens;

    _Strip_Spaces(tokens);

    t->ok = 0;

    if (strlen(tokens) < 8)
    {
        goto cleanup;
    }

    if ((strlen(tokens) > 11) && ((*(tokens + 10) == 'T') || (*(tokens + 10) == 't')))
    {
        tokens += 11;
    }

    int status = sscanf(
        tokens, "%02d:%02d:%02d", &(t->hour), &(t->minute), &(t->second));

    if (status != 3)
    {
        goto cleanup;
    }
    if (t->hour < 0 || t->hour > 23)
    {
        goto cleanup;
    }
    if (t->minute < 0 || t->minute > 59)
    {
        goto cleanup;
    }
    if (t->second < 0 || t->second > 59)
    {
        goto cleanup;
    }

    if (strlen(tokens) == 8)
    {
        t->offset = 0;
        t->ok = 1;
        goto cleanup;
    }
    else
    {
        tokens += 8;
    }

    if (*tokens == '.')
    {
        tokens++;
        char fractions[7] = {0};

        for (unsigned int i = 0; i < 6; i++)
        {
            if ((*(tokens + i) >= 48) && (*(tokens + i) <= 57))
            {
                fractions[i] = *(tokens + i);
            }
            else
            {
                break;
            }
        }

        status = sscanf(fractions, "%d", &(t->fraction));

        if (strlen(fractions) < 6 && strlen(fractions) > 0)
        {
            t->fraction = t->fraction * pow(10, 6 - strlen(fractions)); // convert msec to usec
        }
        else if (strlen(fractions) == 6)
        {
            // all fine, already in usec
        }
        else
        {
            // Invalid fractions must be msec or usec
            goto cleanup;
        }

        if (status != 1)
        {
            goto cleanup;
        }
        if (t->fraction < 0 || t->fraction > 999999)
        {
            goto cleanup;
        }

        tokens += strlen(fractions);

        if (strlen(tokens) == 0)
        {
            t->offset = 0;
            t->ok = 1;
            goto cleanup;
        }
    }

    if ((*tokens == 'Z') || (*tokens == 'z'))
    {
        t->offset = 0;

        tokens++;
        if (strlen(tokens) == 0)
        {
            t->ok = 1;
        }
        else
        {
            t->ok = 0;
        }
        goto cleanup;
    }
    else if ((*tokens == '+') || (*tokens == '-'))
    {
        unsigned int tz_hour, tz_minute;

        status = sscanf(tokens + 1, "%02d:%02d", &tz_hour, &tz_minute);

        if (status != 2)
        {
            goto cleanup;
        }
        if ((tz_hour < 0) || (tz_hour > 23))
        {
            goto cleanup;
        }
        if ((tz_minute < 0) || (tz_minute > 59))
        {
            goto cleanup;
        }

        int tz_offset = (tz_hour * HOUR_IN_MINS) + tz_minute;

        if (*tokens == '-')
        {
            tz_offset = tz_offset * -1;
        }

        t->offset = tz_offset;

        tokens = tokens + 6;
        if (strlen(tokens) == 0)
        {
            t->ok = 1;
        }
        else
        {
            t->ok = 0;
        }
    }

cleanup:
    free(token_ptr);
    tokens = NULL;
    token_ptr = NULL;
    return;
}

void _Qiniu_Parse_Date_Time(char *datetime_string, Qiniu_DateTime *dt)
{
    dt->ok = 0;

    _Qiniu_Parse_Date(datetime_string, &(dt->date));
    if (dt->date.ok == 0)
    {
        return;
    }

    _Qiniu_Parse_Time(datetime_string, &(dt->time));
    if (dt->time.ok == 0)
    {
        return;
    }

    dt->ok = 1;
}
