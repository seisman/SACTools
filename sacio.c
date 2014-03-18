#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sacio.h"

/* function prototype for local use */
void ByteSwap(char *pt, size_t n);
int CheckByteOrder(void);   /* keep it for future use */
int CheckSacHeaderVersion(const int nvhdr);
void map_chdr_in(char *memar, char *buff);
int rsachead (const char *name, SACHEAD *hd, FILE *strm);
void map_chdr_out (char *memar, char *buff);
int wsachead(const char *name, SACHEAD hd, FILE *strm);

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
  -12345 ,      6 , -12345 , -12345 , -12345 ,
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

int ReadSacHead( const char *name, SACHEAD *hd ) 
{
    FILE *strm;
    int lswap;

    if ( (strm = fopen(name, "rb")) ==NULL ) {
        fprintf(stderr, "Unable to open %s\n", name);
        return -1;
    }

    lswap = rsachead (name, hd, strm);

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

float* ReadSac( const char *name, SACHEAD *hd ) 
{
    FILE *strm;
    float *ar;
    int lswap;
    size_t sz;

    if ( (strm = fopen(name, "rb")) == NULL ) {
        fprintf(stderr, "Unable to open %s\n", name);
        return NULL;
    }

    lswap = rsachead(name, hd, strm);

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

    if ( lswap == TRUE ) ByteSwap( (char*) ar, sz);

    return ar;
}

int WriteSac( const char *name, SACHEAD hd, const float *ar ) {
    FILE *strm;
    size_t sz;

    sz = (size_t)hd.npts * SAC_HEADER_SIZEOF_NUMBER;

    if ( (strm = fopen(name, "wb")) == NULL ) {
        fprintf(stderr, "Error in opening file for writing %s\n", name);
        return -1;
    }

    if ( wsachead(name, hd, strm) == -1 ) {
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
    newhdr:

    Description: create a new SAC header with required fields

    IN:
        float   dt  :   sample interval
        int     ns  :   number of points
        float   b0  :   starting time
*******************************************************************************/
/*
SACHEAD newhdr( float dt, int ns, float b0) {
    SACHEAD hd = sac_null;
    hd.delta    =   dt;
    hd.npts     =   ns;
    hd.b        =   b0;
    hd.o        =   0.;
    hd.e        =   b0+(ns-1)*dt;
    hd.iztype   =   IO;
    hd.iftype   =   ITIME;
    hd.leven    =   TRUE;
    return hd;
}
*/

/*******************************************************************************
    byteswap

    IN:
        char    *pt : pointer to byte array
        size_t   m  : number of bytes
    Return: none

    Notes:
        For 4 bytes, 
        byte swapping means taking [0][1][2][3], 
        and turning it into [3][2][1][0]
*******************************************************************************/

void ByteSwap( char *pt, size_t n ) 
{
    size_t i;
    char tmp;
    for (i=0; i<n; i+=4) {
        tmp = pt[i+3];
        pt[i+3] = pt[i];
        pt[i] = tmp;

        tmp = pt[i+2];
        pt[i+2] = pt[i+1];
        pt[i+1] = tmp;
    }
}

#define ENDIAN_BIG 1
#define ENDIAN_LITTLE 0
#define ENDIAN_UNKNOWN -1
/*******************************************************************************
    CheckByteOrder

    Description: Determine the byte order of the machine

    IN: none

    Return: 
        ENDIAN_BIG
        ENDIAN_LITTLE
    
*******************************************************************************/
int CheckByteOrder(void) {
    static int byte_order = ENDIAN_UNKNOWN;
    short int word = 0x0001;
    char *byte = (char *) &word;
    
    if ( byte_order == ENDIAN_UNKNOWN ) {
        byte_order = ( !byte[0] ) ? ENDIAN_BIG : ENDIAN_LITTLE;
    }
    return byte_order;
}

/*******************************************************************************
    CheckSacHeaderVersion

    Description: Determine the byte order of the SAC file

    IN: 
        const int nvhdr : nvhdr from header
    
    Return: 
        FALSE   no byte order swap is needed
        TRUE    byte order swap is needed
        -1      not in sac format ( nvhdr != SAC_HEADER_MAJOR_VERSION )

*******************************************************************************/
int CheckSacHeaderVersion(const int nvhdr) {
    int lswap = FALSE;
    
    if ( nvhdr != SAC_HEADER_MAJOR_VERSION ) {
        ByteSwap( (char*) &nvhdr, sizeof(int) );
        if ( nvhdr != SAC_HEADER_MAJOR_VERSION ) 
            lswap = -1;
        else 
            lswap = TRUE;
    }
    return lswap;
}

void map_chdr_in(char *memar, char *buff) 
{
    char *ptr1, *ptr2;
    int i;

    ptr1 = (char *)memar;
    ptr2 = (char *)buff;

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
    rsachead:
        read sac header in and deal with possible byte swap.
*******************************************************************************/
int rsachead (const char *name, SACHEAD *hd, FILE *strm) {
    int lswap;
    char* buffer;

    // read numeric parts of the SAC header
    if ( fread(hd, SAC_HEADER_NUMBERS_SIZE_BYTES_FILE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC header %s\n", name);
        return -1;
    }

    // Check Header Version and Endian
    lswap = CheckSacHeaderVersion( hd->nvhdr );
    if ( lswap == -1 ) {
        fprintf(stderr, "Warning: %s not in sac format.\n", name);
        return -1;
    } else if ( lswap == TRUE ) {
        ByteSwap( (char *)hd, SAC_HEADER_NUMBERS_SIZE_BYTES_FILE );
    }

    // read string parts of the SAC header
    if ( (buffer = (char *)malloc(SAC_HEADER_STRINGS_SIZE_BYTES_FILE)) == NULL ){
        fprintf(stderr, "Error in allocating memory %s\n", name);
        return -1;
    }
    if ( fread(buffer, SAC_HEADER_STRINGS_SIZE_BYTES_FILE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC header %s\n", name);
        return -1;
    }
    map_chdr_in((char *)(hd)+SAC_HEADER_NUMBERS_SIZE_BYTES_FILE, (char*)buffer);
    free(buffer);

    return lswap;
}

void map_chdr_out (char *memar, char *buff) {
    char *ptr1, *ptr2;
    int i;
    
    ptr1 = (char *)memar;
    ptr2 = (char *)buff;

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

int wsachead(const char *name, SACHEAD hd, FILE *strm) {
    char *buffer;

    if ( fwrite(&hd, SAC_HEADER_NUMBERS_SIZE_BYTES_FILE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
        return -1;
    }

    if ( (buffer = (char *)malloc(SAC_HEADER_STRINGS_SIZE_BYTES_FILE)) == NULL ){
        fprintf(stderr, "Error in allocating memory %s\n", name);
        return -1;
    }
    map_chdr_out((char *)(&hd)+SAC_HEADER_NUMBERS_SIZE_BYTES_FILE, (char *)buffer);

    if ( fwrite(buffer, SAC_HEADER_STRINGS_SIZE_BYTES_FILE, 1, strm) != 1 ) {
        fprintf(stderr, "Error in writing SAC data for writing %s\n", name);
        return -1;
    }

    return 0;
}
