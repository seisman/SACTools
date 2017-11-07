/*
 *  Convert a SAC file to a one/two column table.
 *
 *  Author: Dongdong Tian @ USTC
 *
 *  Revisions:
 *      2014-08-13  Dongdong Tian   Initial Coding.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sacio.h"

void usage(void);

void usage(){
    fprintf(stderr, "Convert a SAC file to a one/two column table.\n");
    fprintf(stderr, "                                             \n");
    fprintf(stderr, "Usage:                                       \n");
    fprintf(stderr, "  sac2col [-C <cols>] [-h] sacifle           \n");
    fprintf(stderr, "                                             \n");
    fprintf(stderr, "Options:                                     \n");
    fprintf(stderr, "  -C <cols>    output data in 1 or 2 column. \n");
    fprintf(stderr, "  -h           show usage.                   \n");
}

int main(int argc, char *argv[])
{
    int c, i;
    int cols = 1;
    char sacfile[80];
    float *xdata = NULL;
    float *ydata = NULL;
    SACHEAD hd;

    while ((c=getopt(argc, argv, "C:h")) != -1) {
        switch (c) {
            case 'C':
                sscanf(optarg, "%d", &cols);
                if (cols!=1 && cols!=2) {
                    fprintf(stderr, "cols is 1 or 2.\n");
                    exit(-1);
                }
                break;
            case 'h':
                usage();
                return -1;
            default:
                return -1;
        }
    }

    if (argc-optind != 1) {
        usage();
        exit(-1);
    }

    strcpy(sacfile, argv[optind]);
    if (read_sac_head(sacfile, &hd)!=0) exit(-1);

    switch (hd.iftype) {
        case ITIME:
            ydata = read_sac(sacfile, &hd);
            if (cols==1) {
                printf("DATA %s %f %f\n", sacfile, hd.delta, hd.b);
                for (i=0; i<hd.npts; i++)
                    printf("%g\n", ydata[i]);
            } else if (cols==2) {
                for (i=0; i<hd.npts; i++)
                    printf("%g %g\n", hd.b+i*hd.delta, ydata[i]);
            }
            break;

        case IXY:
            read_sac_xy(sacfile, &hd, xdata, ydata);
            for (i=0; i<hd.npts; i++)
                printf("%g %g\n", xdata[i], ydata[i]);
            break;

        default:
            fprintf(stderr, "%s is not ITIME/IXY type\n", sacfile);
            break;
    }

    return 0;
}
