#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "datetime.h"

static int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31};

int ymd2doy(int year, int month, int day)
{
    int i;
    int doy = 0;

    for (i=0; i<month-1; i++) {
        doy += days_in_month[i];
        if(i==1 && ISLEAP(year)) doy++;
    }
    doy += day;

    return doy;
}

void doy2ymd(int year, int doy, int *month, int *day)
{
    int i, dim, leap;

    leap = ISLEAP(year);
    *day = doy;
    for (i=0; i<12; i++) {
        dim = days_in_month[i];
        if(leap && i==1) dim++;
        if(*day <= dim) break;
        *day -= dim;
    }
    *month = i + 1;
}

/* convert julian date to epoch time */
double day2epoch(int year, int doy)
{
    long cnt;
    long days;

    cnt = (long)year;
    days = 0L;
    if (cnt > 1970L) {
        while (--cnt >= 1970L)
            days += ISLEAP(cnt) ? 366 : 365;
    } else if (cnt < 1970L) {
        while (cnt < 1970L) {
            days -= ISLEAP(cnt) ? 366 : 365;
            cnt++;
        }
    }
    return((days + ((doy - 1))) * 86400.);
}

/* convert from human to epoch */
double datetime2epoch(int year, int jday, int hour, int min, int sec, int msec)
{
    double epoch;

    epoch = day2epoch(year, jday)
          + hour * 3600.
          + min * 60.
          + sec
          + 0.001*msec;

    return epoch;
}

#define mod(a,b) (a) - ((int)((a)/(b))) * (b)
void epoch2datetime(double epoch,
                    int *year, int *doy, int *month, int *day,
                    int *hour, int *minute, int *second, int *msec)
{
    int diy;
    double secleft;

    *doy = (int)(epoch / 86400.);
    secleft = mod(epoch,86400.0);
    *hour = *minute = *second = *msec = 0;

    if(secleft != 0.0) {        /* compute hours minutes seconds */
        if(secleft < 0) {   /* before 1970 */
            *doy = *doy - 1;        /* subtract a day */
            secleft += 86400;   /* add a day */
        }
        *hour = (int)(secleft/3600);
        secleft = fmod(secleft,3600.0);
        *minute = (int)(secleft/60);
        *second = (int)(fmod(secleft,60.0));
        secleft = fmod(secleft,60.0) - (*second);
        *msec = (int)(1000*(secleft+0.00049));
    }

    if(*doy >= 0){
        for( *year = 1970 ; ; (*year)++ ){
            diy = ISLEAP(*year) ? 366:365;
            if( *doy < (long)diy ) break;
            (*doy) -= (long)diy;
        }
    } else {
        for( *year = 1969 ; ; (*year)-- ){
            diy = ISLEAP(*year) ? 366:365;
            *doy += (long)diy;
            if( *doy >= 0L ) break;
        }
    }
    *doy = *doy + 1L;
    doy2ymd(*year, *doy, month, day);
}

DATETIME datetime_new(int year, int month, int day,
        int hour, int min, int sec, int msec)
{
    DATETIME dt;

    dt.year = year;
    dt.month = month;
    dt.day = day;
    dt.hour = hour;
    dt.minute = min;
    dt.second = sec;
    dt.msec = msec;

    dt.doy = ymd2doy(year, month, day);
    dt.epoch = datetime2epoch(year, dt.doy, hour, min, sec, msec);

    return dt;
}
