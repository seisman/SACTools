/*
 *  List the values of selected head fields
 *
 *  Author: Dongdong Tian @ USTC
 *
 *  Revision:
 *    2014-08-26  Dongdong Tian     Initial Coding
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sacio.h"

void usage(void);

void usage()
{
    fprintf(stderr, "List the values of selected head fields                \n");
    fprintf(stderr, "                                                       \n");
    fprintf(stderr, "Usage:                                                 \n");
    fprintf(stderr, "  saclh -H head_fields_lists [-N] sacfiles             \n");
    fprintf(stderr, "                                                       \n");
    fprintf(stderr, "Options:                                               \n");
    fprintf(stderr, "  -N:  do not output filename in colunm 1              \n");
    fprintf(stderr, "                                                       \n");
    fprintf(stderr, "Note:                                                  \n");
    fprintf(stderr, "  1. lists should be seperated by commas               \n");
    fprintf(stderr, "                                                       \n");
    fprintf(stderr, "Examples:                                              \n");
    fprintf(stderr, "  saclh -H evla,evlo,stla,stlo seis1 seis2             \n");
    fprintf(stderr, "  saclh -H evla -N seis                                \n");
}

int main(int argc, char *argv[])
{
    int c;
    char *p;
    int head[20];
    int cnt = 0;
    int noname = 0;

    int i;

    while ((c=getopt(argc, argv, "H:Nh")) != -1) {
        switch (c) {
            case 'H':
                p = strtok(optarg, ",/");
                while (p != NULL) {
                    head[cnt] = sac_head_index(p);
                    if (head[cnt] < 0) {
                        fprintf(stderr, "Error in sac head name: %s\n", p);
                        exit(-1);
                    }
                    cnt++;
                    p = strtok(NULL, ",/");
                }
                break;
            case 'N':
                noname = 1;
                break;
            case 'h':
                usage();
                return -1;
            default:
                return -1;
        }
    }

    if (argc-optind == 0) {
        usage();
        exit(-1);
    }

    int j;
    SACHEAD hd;

    /* special case:
     *    if only one file, one head field, and output no filename,
     *    omit the newline so that it's easier for scripts to handle output.
     */
    int newline = (argc-optind==1 && cnt==1 && noname);

    for (i=optind; i<argc; i++) {   /* loop over files */
        if ((read_sac_head(argv[i], &hd)) != 0) continue;

        if (noname==0) printf("%s ", argv[i]);
        for (j=0; j<cnt; j++) {
            if (head[j] < SAC_HEADER_FLOATS) {
                float *pt = &hd.delta;
                printf("%g ", *(pt + head[j]));
            } else if (head[j] < SAC_HEADER_NUMBERS) {
                int *pt = &hd.nzyear;
                printf("%d ", *(pt + head[j] - SAC_HEADER_FLOATS));
            } else {
                char *pt = hd.kstnm;
                int offset = (head[j]-SAC_HEADER_NUMBERS)*SAC_HEADER_STRING_LENGTH;
                printf("%s ", pt + offset);
            }
        }
        if (!newline) printf("\n");
    }

    return 0;
}
