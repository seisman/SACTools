/*
 *  Change the values of selected head fields
 *
 *  Author: Dongdong Tian @ USTC
 *
 *  Revision:
 *      2014-08-23  Dongdong Tian   Initial Coding
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sacio.h"

void usage(void);

void usage() {
    fprintf(stderr, "Change the value of selected head fields       \n");
    fprintf(stderr, "                                               \n");
    fprintf(stderr, "Usgae:                                         \n");
    fprintf(stderr, "   sacch key1=value1 key2=value2 ... sacfiles  \n");
    fprintf(stderr, "   sacch time=year-month-dayThour:min:sec.msec sacfiles\n");
    fprintf(stderr, "                                               \n");
    fprintf(stderr, "Notes:                                         \n");
    fprintf(stderr, "   1. keys are sac head fields, like npts, evla\n");
    fprintf(stderr, "   2. values are integers, floats or strings   \n");
    fprintf(stderr, "   3. key=undef to set key to undefinded value.\n");
    fprintf(stderr, "                                               \n");
    fprintf(stderr, "Examples:                                      \n");
    fprintf(stderr, "   sacch stla=10.2 stlo=20.2 kstnm=COLA seis1 seis2 \n");
    fprintf(stderr, "   sacch time=2010-02-03T10:20:35.200  seis1 seis2  \n");
    fprintf(stderr, "   sacch t9=undef kt9=undef seis*                   \n");
}

#define MAX_HEAD 20
int main(int argc, char *argv[])
{
    struct {
        int index;
        float value;
    } Fkeyval[MAX_HEAD];

    struct {
        int index;
        int value;
    } Ikeyval[MAX_HEAD];

    struct {
        int offset;
        char value[80];
    } Ckeyval[MAX_HEAD];

    int i, j;
    char key[10];
    char val[80];
    int year, month, day, hour, min, sec, msec;
    float secs;
    char *p;
    int time = 0;
    int fkey = 0;
    int ikey = 0;
    int ckey = 0;
    int file = 0;
    float *data;
    char sacfile[80];
    SACHEAD hd;


    int cal2jul(int year, int month, int day);

    char args[80];
    for (i=1; i<argc; i++) {
        strcpy(args, argv[i]);
        if ((p = strchr(args, '=')) != NULL) {   /* key=value pairs */
            *p = ' ';
            sscanf(args, "%s %s", key, val);
            if (strcasecmp(key, "time") == 0) {
                time = 1;
                if (strcasecmp(val, "undef") == 0) {
                    year = SAC_INT_UNDEF;
                    month = SAC_INT_UNDEF;
                    day = SAC_INT_UNDEF;
                    hour = SAC_INT_UNDEF;
                    min = SAC_INT_UNDEF;
                    sec = SAC_INT_UNDEF;
                    msec = SAC_INT_UNDEF;
                } else {
                    j = sscanf(val, "%d-%d-%dT%d:%d:%f",
                               &year, &month, &day, &hour, &min, &secs);
                    if (j != 6) {
                        fprintf(stderr, "Error in time format\n");
                        exit(-1);
                    }
                    sec  = floor(secs);
                    msec = (int)((secs - (float)sec) * 1000 + 0.5);
                }
            } else {
                int index = sac_head_index(key);
                if (index < 0) {
                    fprintf(stderr, "Error in sac head name: %s\n", key);
                    exit(-1);
                } else if (index >=0 && index < SAC_HEADER_FLOATS) {
                    Fkeyval[fkey].index = index;
                    if (strcasecmp(val, "undef") == 0)
                        Fkeyval[fkey].value = SAC_FLOAT_UNDEF;
                    else
                        Fkeyval[fkey].value = atof(val);
                    fkey++;
                } else if (index < SAC_HEADER_NUMBERS) {
                    /* index relative to the start of int fields */
                    Ikeyval[ikey].index = index - SAC_HEADER_FLOATS;
                    if (strcasecmp(val, "undef") == 0)
                        Ikeyval[ikey].value = SAC_INT_UNDEF;
                    else
                        Ikeyval[ikey].value = atoi(val);
                    ikey++;
                } else {
                    /* offset in bytes relative to the start of */
                    Ckeyval[ckey].offset =
                        (index - SAC_HEADER_NUMBERS) * SAC_HEADER_STRING_LENGTH;
                    if (strcasecmp(val, "undef") == 0) {  /* undefined chars */
                        if (strcasecmp(key, "kevnm") == 0)
                            strcpy(val, SAC_CHAR16_UNDEF);
                        else
                            strcpy(val, SAC_CHAR8_UNDEF);
                    }
                    strcpy(Ckeyval[ckey].value, val);
                    ckey++;
                }
            }
        } else
            file++;
    }

    if (!(time || ikey || fkey || ckey) || !file) {
        usage();
        exit(-1);
    }

    for (i=1; i<argc; i++) {
        /* skip key=value pairs */
        if ((strchr(argv[i], '=')) != NULL) continue;

        strcpy(sacfile, argv[i]);
        if ((data= read_sac(sacfile, &hd)) == NULL) continue;

        for (j=0; j<fkey; j++) {
            float *pt = &hd.delta;
            *(pt + Fkeyval[j].index) = Fkeyval[j].value;
        }
        for (j=0; j<ikey; j++) {
            int *pt = &hd.nzyear;
            *(pt + Ikeyval[j].index) = Ikeyval[j].value;
        }
        for (j=0; j<ckey; j++) {
            char *pt = hd.kstnm;
            strcpy(pt+Ckeyval[j].offset, Ckeyval[j].value);
        }
        if (time) {
            int jday;
            jday = cal2jul(year, month, day);

            hd.nzyear = year;
            hd.nzjday = jday;
            hd.nzhour = hour;
            hd.nzmin  = min;
            hd.nzsec  = sec;
            hd.nzmsec = msec;
        }
        write_sac(sacfile, hd, data);
        free(data);
    }
    return 0;
}

/* convert month and day to jday */
int cal2jul(int year, int month, int day) {
    int noleap[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int   leap[12] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

    if ((year%4==0 && year%100!=0) || (year%400==0))
        return leap[month-1] + day;
    else
        return noleap[month-1] + day;
}
