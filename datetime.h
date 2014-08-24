#ifndef _DATETIME_H
#define _DATETIME_H

#define ISLEAP(yr) ((!((yr) % 4) && (yr) % 100) || !((yr) % 400))

typedef struct date_time {
    int year;
    int month;
    int day;
    int doy;
    int hour;
    int minute;
    int second;
    int msec;
    double epoch;
} DATETIME;

int ymd2doy(int year, int month, int day);
void doy2ymd(int year, int doy, int *month, int *day);
double day2epoch(int year, int doy);
double datetime2epoch(int year, int doy, int hour, int min, int sec, int msec);
void epoch2datetime(double epoch,int *year,int *doy,int *month,int *day,int *hour,int *minute,int *second,int *msec);
DATETIME datetime_new(int year, int month, int day, int hour, int min, int sec, int msec);

#endif
