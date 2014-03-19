/*******************************************************************************
 *                                  sacio.c
 *  SAC I/O functions:
 *      ReadSacHead     read SAC header
 *      ReadSac         read SAC binary data
 *      ReadSacPdw      read SAC data in a partial data window ( cut option )
 *      WriteSac        Write SAC binary data
 *      NewSacHead      Create a new minimal SAC header
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sacio.h"

/* function prototype for local use */
void    byte_swap       (char *pt, size_t n);
int     check_sac_nvhdr (const int nvhdr);
void    map_chdr_in     (char *memar, char *buff);
int     read_sac_head   (const char *name, SACHEAD *hd, FILE *strm);
void    map_chdr_out    (char *memar, char *buff);
int     write_sac_head  (const char *name, SACHEAD hd, FILE *strm);

/* a SAC structure containing all null values */
static SACHEAD sac_null = {
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345., -12345., -12345., -12345., -12345.,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  -12345 , -12345 , -12345 , -12345 , -12345 ,
  { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
  { '-','1','2','3','4','5',' ',' ' }
};

/*******************************************************************************
    ReadSacHead

    Description: Read binary SAC header from file.

    IN: 
        const char *name : File name
    OUT:
        SACHEAD    *hd   : SAC header

    Return: 0 if success, -1 if failed

*******************************************************************************/

int ReadSacHead( const char *name, SACHEAD *hd ) {
    FILE    *strm;
    int     lswap;

    if ( (strm = fopen(name, "rb")) == NULL ) {
        fprintf(stderr, "Unable to open %s\n", name);
        return -1;
    }

    lswap = read_sac_head (name, hd, strm);

    fclose(strm);

    return ( (lswap == -1) ? -1 : 0 );
}

/*******************************************************************************
    ReadSac

    Description: Read binary SAC data from file.

    IN: 
        const char *name : file name
    OUT:
        SACHEAD    *hd   : SAC header to be filled
    Return: float pointer to the data array, NULL if failed.

*******************************************************************************/

float* ReadSac( const char *name, SACHEAD *hd ) {
    FILE    *strm;
    float   *ar;
    int     lswap;
    size_t  sz;

    if ( (strm = fopen(name, "rb")) == NULL ) {
        fprintf(stderr, "Unable to open %s\n", name);
        return NULL;
    }

    lswap = read_sac_head(name, hd, strm);

    if ( lswap == -1 ) {
        fclose(strm);
        return NULL;
    }
    
    sz = (size_t) hd->npts * SAC_HEADER_SIZEOF_NUMBER;
    if ( (ar = (float *)malloc(sz) ) == NULL ) {
        fprintf(stderr, "Error in allocating memory for reading %s\n", name);
        fclose(strm);
        return NULL;
    }

    if ( fread((char*)ar, sz, 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC data %s\n", name);
        fclose(strm);
        return NULL;
    }
    fclose(strm);

    if ( lswap == TRUE ) byte_swap( (char*) ar, sz);

    return ar;
}

/*******************************************************************************
    WriteSac

    Description:    write binary SAC data

    IN: 
        const char *name    :   file name
        SACHEAD     hd      :   header
        const float *ar     :   float data array

    Return:
        -1  :   fail
        0   :   succeed   
*******************************************************************************/
int WriteSac( const char *name, SACHEAD hd, const float *ar ) {
    FILE    *strm;
    size_t  sz;

    sz = (size_t)hd.npts * SAC_HEADER_SIZEOF_NUMBER;

    if ( (strm = fopen(name, "wb")) == NULL ) {
        fprintf(stderr, "Error in opening file for writing %s\n", name);
        return -1;
    }

    if ( write_sac_head(name, hd, strm) == -1 ) {
        fclose(strm);
        return -1;
    }

    if ( fwrite(ar, sz, 1, strm) != 1 ) {
        fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
        fclose(strm);
        return -1;
    } 
    fclose(strm);
    return 0;
}

/*******************************************************************************
    ReadSacPwd

    Description:
        Read portion of data from file.

    Arguments:
        const char  *name   :   file name
        SACHEAD     *hd     :   SAC header to be filled
        int         tmark   :   time mark in SAC header
                                    -5  ->  b;
                                    -4  ->  e;
                                    -3  ->  o;
                                    -2  ->  a;
                                    0-9 ->  Tn;
                                    others -> t=0;
        float       t1      :   begin time is tmark + t1
        float       t2      :   end time is tmark + t2

    Return:
        float pointer to the data array, NULL if failed.
        
*******************************************************************************/
float *ReadSacPwd(const char *name, SACHEAD *hd, int tmark, float t1, float t2) {
    FILE    *strm;
    int     lswap;
    int     nn;
    float   tref;
    int     nt1;
    int     nt2;
    float   *ar;
    float   *fpt;
    int     npts;

    if ( (strm = fopen(name, "rb")) == NULL) {
        fprintf(stderr, "Error in opening %s\n", name);
        return NULL;
    }

    lswap = read_sac_head(name, hd, strm);

    if ( lswap == -1 ) {
        fclose(strm);
        return NULL;
    }

    nn = (int) ( (t2-t1) / hd->delta );
    if ( nn<=0 || (ar = (float *)calloc((size_t)nn, SAC_DATA_SIZEOF)) == NULL) {
        fprintf(stderr, "Errorin allocating memory for reading %s n=%d\n", name, nn);
        return NULL;
    }

    tref = 0.;
    if ( (tmark>=-5&&tmark<=-2) || (tmark>=0 && tmark<=9) ) {
        tref = *((float *) hd + TMARK + tmark);
        if (fabs(tref+12345.)<0.1){
            fprintf(stderr, "Time mark undefined in %s\n", name);
            return NULL;
        }
    } 
    t1 += tref;
    nt1 = (int) ( ( t1 - hd->b ) / hd->delta );
    nt2 = nt1 + nn;
    npts = hd->npts;
    hd->npts = nn;
    hd->b   = t1;
    hd->e   = t1 + nn * hd->delta;

    if ( nt1>npts || nt2 <0 ) return ar;    /* return zero filled array */
    /* maybe warnings are needed! */
    
    if ( nt1<0 ) {
        fpt = ar - nt1;
        nt1 = 0;
    } else {
        if ( fseek(strm, nt1*SAC_HEADER_SIZEOF_NUMBER, SEEK_CUR) < 0 ) {
            fprintf(stderr, "Error in seek %s\n", name);
            fclose(strm);
            return NULL;
        }
        fpt = ar;
    }
    if (nt2>npts) nt2 = npts;
    nn = nt2 - nt1;

    if ( fread((char *)fpt, (size_t)nn * SAC_DATA_SIZEOF, 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC data %s\n", name);
        fclose(strm);
        return NULL;
    }
    fclose(strm);

    if ( lswap == TRUE ) byte_swap( (char*) ar, (size_t)nn*SAC_DATA_SIZEOF);

    return ar;
}

/*******************************************************************************
    NewSacHead:

    Description: create a new SAC header with required fields

    IN:
        float   dt  :   sample interval
        int     ns  :   number of points
        float   b0  :   starting time
*******************************************************************************/
SACHEAD NewSacHead ( float dt, int ns, float b0) {
    SACHEAD hd = sac_null;
    hd.delta    =   dt;
    hd.npts     =   ns;
    hd.b        =   b0;
    hd.o        =   0.;
    hd.e        =   b0+(ns-1)*dt;
    hd.iztype   =   IO;
    hd.iftype   =   ITIME;
    hd.leven    =   TRUE;
    hd.nvhdr    =   SAC_HEADER_MAJOR_VERSION;
    return hd;
}

/******************************************************************************
 *                                                                            *
 *              Functions below are only for local use!                       *
 *                                                                            *
 ******************************************************************************/ 

/*******************************************************************************
    byte_swap : reverse the byte order of 4 bytes int/float.

    IN:
        char    *pt : pointer to byte array
        size_t   m  : number of bytes
    Return: none

    Notes:
        For 4 bytes, 
        byte swapping means taking [0][1][2][3], 
        and turning it into [3][2][1][0]
*******************************************************************************/

void byte_swap ( char *pt, size_t n ) {
    size_t  i   ;
    char    tmp ;
    for (i=0; i<n; i+=4) {
        tmp     =   pt[i+3];
        pt[i+3] =   pt[i];
        pt[i]   =   tmp;

        tmp     =   pt[i+2];
        pt[i+2] =   pt[i+1];
        pt[i+1] =   tmp;
    }
}

/*******************************************************************************
    check_sac_nvhdr

    Description: Determine the byte order of the SAC file

    IN: 
        const int nvhdr : nvhdr from header
    
    Return: 
        FALSE   no byte order swap is needed
        TRUE    byte order swap is needed
        -1      not in sac format ( nvhdr != SAC_HEADER_MAJOR_VERSION )

*******************************************************************************/

int check_sac_nvhdr ( const int nvhdr ) {
    int lswap = FALSE;
    
    if ( nvhdr != SAC_HEADER_MAJOR_VERSION ) {
        byte_swap( (char*) &nvhdr, SAC_HEADER_SIZEOF_NUMBER );
        if ( nvhdr != SAC_HEADER_MAJOR_VERSION ) 
            lswap = -1;
        else 
            lswap = TRUE;
    }
    return lswap;
}

/*******************************************************************************
    map_chdr_in:
        map strings from buffer to memory

*******************************************************************************/

void map_chdr_in ( char *memar, char *buff ) {
    char    *ptr1;
    char    *ptr2;
    int     i;

    ptr1 = memar;
    ptr2 = buff;

    memcpy(ptr1, ptr2, 8);
    *(ptr1+8) = '\0';
    ptr1 += 9;
    ptr2 += 8;

    memcpy(ptr1, ptr2, 16);
    *(ptr1+16) = '\0';
    ptr1 += 18;
    ptr2 += 16;

    for ( i=0; i<21; i++) {
        memcpy(ptr1, ptr2, 8);
        *(ptr1+8) = '\0';
        ptr1 += 9;
        ptr2 += 8;
    }

    return;
}

/*******************************************************************************
    read_sac_head:
        read sac header in and deal with possible byte swap.

    IN:
        const char *name : file name, only for debug
        SACHEAD    *hd   : header to be filled
        FILE       *strm : file handler

    Return:
        0   :   Succeed and no byte swap
        1   :   Succeed and byte swap
        -1  :   fail.
*******************************************************************************/
int read_sac_head (const char *name, SACHEAD *hd, FILE *strm) {
    char*   buffer;
    int     lswap;

    /* read numeric parts of the SAC header */
    if ( fread(hd, SAC_HEADER_NUMBERS_SIZE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC header %s\n", name);
        return -1;
    }

    /* Check Header Version and Endian  */
    lswap = check_sac_nvhdr( hd->nvhdr );
    if ( lswap == -1 ) {
        fprintf(stderr, "Warning: %s not in sac format.\n", name);
        return -1;
    } else if ( lswap == TRUE ) 
        byte_swap( (char *)hd, SAC_HEADER_NUMBERS_SIZE );

    /* read string parts of the SAC header */
    if ( (buffer = (char *)malloc(SAC_HEADER_STRINGS_SIZE)) == NULL ){
        fprintf(stderr, "Error in allocating memory %s\n", name);
        return -1;
    }
    if ( fread(buffer, SAC_HEADER_STRINGS_SIZE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC header %s\n", name);
        return -1;
    }
    map_chdr_in((char *)(hd)+SAC_HEADER_NUMBERS_SIZE, buffer);
    free(buffer);

    return lswap;
}

/*******************************************************************************
    map_chdr_out:
        map strings from memory to string

*******************************************************************************/
void map_chdr_out ( char *memar, char *buff ) {
    char    *ptr1;
    char    *ptr2;
    int     i;
    
    ptr1 = memar;
    ptr2 = buff;

    memcpy(ptr2, ptr1, 8);
    ptr1 += 9;
    ptr2 += 8;

    memcpy(ptr2, ptr1, 16);
    ptr1 += 18;
    ptr2 += 16;

    for ( i=0; i<21; i++) {
        memcpy(ptr2, ptr1, 8);
        ptr1 += 9;
        ptr2 += 8;
    }

    return;
}
/*******************************************************************************
    write_sac_head

    IN:
        const char *name : file name, only for debug
        SACHEAD     hd   : header to be written
        FILE       *strm : file handler

    Return:
        -1  :   failed.
        0   :   success.
 
*******************************************************************************/
int write_sac_head(const char *name, SACHEAD hd, FILE *strm) {
    char *buffer;

    if ( fwrite(&hd, SAC_HEADER_NUMBERS_SIZE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
        return -1;
    }

    if ( (buffer = (char *)malloc(SAC_HEADER_STRINGS_SIZE)) == NULL ){
        fprintf(stderr, "Error in allocating memory %s\n", name);
        return -1;
    }
    map_chdr_out((char *)(&hd)+SAC_HEADER_NUMBERS_SIZE, buffer);

    if ( fwrite(buffer, SAC_HEADER_STRINGS_SIZE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
        return -1;
    }
    free(buffer);

    return 0;
}
