/*
 *  Get max amplitude of SAC files in a specified time window
 *
 *  Author: Dongdong Tian
 *
 *  Revision:
 *    2017-11-07  Dongdong Tian   Initial coding.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <float.h>
#include <math.h>
#include "sacio.h"

void usage(void);

void usage() {
    fprintf(stderr, "Get max amplitude of SAC files in a specified time window.\n");
    fprintf(stderr, "                                                          \n");
    fprintf(stderr, "Usage:                                                    \n");
    fprintf(stderr, "  sacmax [-Mmode] [-Ttmark/t0/t1] sacfiles                \n");
    fprintf(stderr, "                                                          \n");
    fprintf(stderr, "Options:                                                  \n");
    fprintf(stderr, "  -M0   return maximum amplitude                          \n");
    fprintf(stderr, "  -M1   return minumum amplitude                          \n");
    fprintf(stderr, "  -M2   return maximum absolute amplitude                 \n");
    fprintf(stderr, "  -M3   return absolute maximum amplitude                 \n");
    fprintf(stderr, "  -M4   return maximum peak-to-peak amplitude             \n");
    fprintf(stderr, "  -T    specify time window.                              \n");
    fprintf(stderr, "  -h    show usage.                                       \n");
}

int main(int argc, char *argv[])
{
    int c;
    int mode;
    int error;
    int cut = 0;   /* cut a time window or not */
    int tmark;
    float t0, t1;
    int i;

    error = 0;
    while ((c=getopt(argc, argv, "M:T:h")) != -1) {
        switch (c) {
            case 'M':
                if (sscanf(optarg, "%d", &mode) != 1) error++;
                if (mode<0 || mode>4) {
                    fprintf(stderr, "ERROR: mode is 0, 1, 2, 4.\n");
                    error++;
                }
                break;
            case 'T':
                if (sscanf(optarg, "%d/%f/%f", &tmark, &t0, &t1) != 3) {
                    error++;
                } else {
                    cut = 1;
                }
                break;
            case 'h':
                usage();
                return -1;
        }
    }

    if (argc-optind < 1 || error) {
        usage();
        exit(-1);
    }


    for (i=optind; i<argc; i++) {  /* loop over files */
        float *data;
        SACHEAD hd;
        int j;

        if (cut) data = read_sac_pdw(argv[i], &hd, tmark, t0, t1);
        else     data = read_sac(argv[i], &hd);

        float value;
        if (mode == 0) {  /* maximum amplitude */
            value = FLT_MIN;  /* initialization */
            for (j=0; j<hd.npts; j++) {
                if (data[j] > value)  value = data[j];
            }
        } else if (mode == 1) { /* minumum amplitude */
            value = FLT_MAX; /* initialization */
            for (j=0; j<hd.npts; j++) {
                if (data[j] < value)  value = data[j];
            }
        } else if (mode == 2) { /* maximum absolute amplitude */
            value = 0;  /* initialization */
            for (j=0; j<hd.npts; j++) {
                if (fabs(data[j]) > fabs(value)) value = fabs(data[j]);
            }
        } else if (mode == 3) { /* absolute maximum amplitude */
            value = 0;  /* initialization */
            for (j=0; j<hd.npts; j++) {
                if (fabs(data[j]) > fabs(value)) value = data[j];
            }
        } else if (mode == 4) { /* maximum peak-to-peak amplitude */
            float value_pos = FLT_MIN;
            float value_neg = FLT_MAX;
            for (j=0; j<hd.npts; j++) {
                if (data[j] > value_pos)  value_pos = data[j];
                if (data[j] < value_neg)  value_neg = data[j];
            }
            value = fabs(value_pos - value_neg);
        }

        printf("%s %g\n", argv[i], value);
    }

    return 0;
}
