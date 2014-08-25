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
#include "datetime.h"

void usage(void);
void datetime_undef(DATETIME *dt);
DATETIME datetime_read(char *string);
DATETIME datetime_set_ref(SACHEAD hd);

void usage() {
    fprintf(stderr, "Change the value of selected head fields       \n");
    fprintf(stderr, "                                               \n");
    fprintf(stderr, "Usgae:                                         \n");
    fprintf(stderr, "   sacch key1=value1 key2=value2 ... sacfiles  \n");
    fprintf(stderr, "   sacch time=DATETIME sacfiles                \n");
    fprintf(stderr, "                                               \n");
    fprintf(stderr, "Notes:                                         \n");
    fprintf(stderr, "   1. keys are sac head fields, like npts, evla\n");
    fprintf(stderr, "   2. values are integers, floats or strings   \n");
    fprintf(stderr, "   3. key=undef to set key to undefinded value.\n");
    fprintf(stderr, "   4. DATETIME format: yyyy-mm-ddThh:mm:ss.mmm \n");
    fprintf(stderr, "   5. variables for time offset can use value  \n");
    fprintf(stderr, "      in DATETIME format                       \n");
    fprintf(stderr, "                                               \n");
    fprintf(stderr, "Examples:                                      \n");
    fprintf(stderr, "   sacch stla=10.2 stlo=20.2 kstnm=COLA seis1 seis2 \n");
    fprintf(stderr, "   sacch time=2010-02-03T10:20:35.200 seis1 seis2   \n");
    fprintf(stderr, "   sacch t7=2010-02-03T10:20:30.000 seis1           \n");
    fprintf(stderr, "   sacch t9=undef kt9=undef seis*                   \n");
}

#define MAX_HEAD 20
int main(int argc, char *argv[])
{
    struct {
        int index;
        int tmark;      // flag for value in DATETIME format or not
        double value;
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
    DATETIME dt;
    char *p;
    int time = 0;
    int fkey = 0;
    int ikey = 0;
    int ckey = 0;
    int file = 0;
    float *data;
    char sacfile[80];
    SACHEAD hd;

    char args[80];
    for (i=1; i<argc; i++) {
        strcpy(args, argv[i]);

        if ((p = strchr(args, '=')) == NULL) {
            /* no equal in argument, assume it is a SAC file */
            file++;
            continue;
        }

        /* KEY=VALUE pairs */
        *p = ' ';  sscanf(args, "%s %s", key, val);
        if (strcasecmp(key, "time") == 0) {     /* TIME */
            time = 1;
            if (strcasecmp(val, "undef") == 0)
                datetime_undef(&dt);
            else
                dt = datetime_read(val);
        } else {                                /* HEAD */
            int index = sac_head_index(key);
            if (index < 0) {
                fprintf(stderr, "Error in sac head name: %s\n", key);
                exit(-1);
            } else if (index >=0 && index < SAC_HEADER_FLOATS) {
                Fkeyval[fkey].index = index;
                Fkeyval[fkey].tmark = 0;
                if (strcasecmp(val, "undef") == 0) {
                    Fkeyval[fkey].value = SAC_FLOAT_UNDEF;
                } else if ((strchr(val, 'T')) != NULL) {
                    /* support datetime for time variables */
                    DATETIME tvalue;
                    tvalue = datetime_read(val);
                    Fkeyval[fkey].value = tvalue.epoch;
                    Fkeyval[fkey].tmark = 1;
                } else {
                    Fkeyval[fkey].value = atof(val);
                }
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
    }

    if (!(time || ikey || fkey || ckey) || !file) {
        usage();
        exit(-1);
    }

    for (i=1; i<argc; i++) {
        DATETIME tref;

        /* skip key=value pairs */
        if ((strchr(argv[i], '=')) != NULL) continue;

        strcpy(sacfile, argv[i]);
        if ((data= read_sac(sacfile, &hd)) == NULL) continue;

        tref = datetime_set_ref(hd);

        for (j=0; j<fkey; j++) {
            float *pt = &hd.delta;
            if (Fkeyval[j].tmark==0) {
                *(pt + Fkeyval[j].index) = (float)Fkeyval[j].value;
            } else if (Fkeyval[j].tmark==1) {
                *(pt + Fkeyval[j].index) = (float)(Fkeyval[j].value - tref.epoch);
            }
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
            hd.nzyear = dt.year;
            hd.nzjday = dt.doy;
            hd.nzhour = dt.hour;
            hd.nzmin  = dt.minute;
            hd.nzsec  = dt.second;
            hd.nzmsec = dt.msec;
        }
        write_sac(sacfile, hd, data);
        free(data);
    }
    return 0;
}

DATETIME datetime_read(char *string)
{
    int num;
    float secs;
    int year, month, day, hour, minute, second, msec;

    num = sscanf(string, "%d-%d-%dT%d:%d:%f",
                 &year, &month, &day, &hour, &minute, &secs);

    if (num != 6) {
        fprintf(stderr, "Error in time format\n");
        exit(-1);
    }
    second  = (int)(floor(secs));
    msec = (int)((secs - (float)second) * 1000 + 0.5);

    return datetime_new(year, month, day, hour, minute, second, msec);
}

void datetime_undef(DATETIME *dt)
{
    dt->year = SAC_INT_UNDEF;
    dt->month = SAC_INT_UNDEF;
    dt->day = SAC_INT_UNDEF;
    dt->doy = SAC_INT_UNDEF;
    dt->hour = SAC_INT_UNDEF;
    dt->minute = SAC_INT_UNDEF;
    dt->second = SAC_INT_UNDEF;
    dt->msec = SAC_INT_UNDEF;
}

DATETIME datetime_set_ref(SACHEAD hd)
{
    int month, day;
    doy2ymd(hd.nzyear, hd.nzjday, &month, &day);

    return datetime_new(hd.nzyear, month, day,
                        hd.nzhour, hd.nzmin, hd.nzsec, hd.nzmsec);
}
