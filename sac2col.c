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
    float *data;
    SACHEAD hd;

    if (argc==1) {
        usage();
        return -1;
    }

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

    if (argc-optind == 0) {
        fprintf(stderr, "Error: one SAC file is needed.\n");
        return -1;
    } else if (argc-optind > 1) {
        fprintf(stderr, "Error: only one SAC file is needed.\n");
        return -1;
    }

    strcpy(sacfile, argv[optind]);
    if ((data = read_sac(sacfile, &hd)) == NULL)
        exit(-1);

    switch (hd.iftype) {
        case ITIME:
            if (cols==1) {
                printf("DATA %s %f %f\n", sacfile, hd.delta, hd.b);
                for (i=0; i<hd.npts; i++)
                    printf("%e\n", data[i]);
            } else if (cols==2) {
                float time = hd.b;
                for (i=0; i<hd.npts; i++) {
                    printf("%e %e\n", time, data[i]);
                    time += hd.delta;
                }
            }
            break;

        case IXY:
            for (i=0; i<hd.npts; i++)
                printf("%e %e\n", data[i], data[i+hd.npts]);
            break;

        default:
            fprintf(stderr, "%s is not ITIME/IXY type\n", sacfile);
            break;
    }

    free(data);
    return 0;
}
