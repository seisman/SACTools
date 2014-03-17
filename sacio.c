#include <stdio.h>
#include <stdlib.h>
#include "sacio.h"


/* function prototype for local use */
void ByteSwap(char *pt, size_t n);
int CheckByteOrder(void);   /* keep it for future use */
int CheckSacHeaderVersion(const int nvhdr);

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

    if ( fread(hd, sizeof(SACHEAD), 1, strm) != 1) {
        fprintf(stderr, "Error in reading SAC header %s\n", name);
        fclose(strm);
        return -1;
    }

    lswap = CheckSacHeaderVersion( hd->nvhdr );

    if ( lswap == -1 ) {
        fprintf(stderr, "Warning: %s not in sac format.\n", name);
        return -1;
    } else if ( lswap == TRUE ) {
        ByteSwap( (char *)hd, HD_SIZE );
    }

    fclose(strm);
    return 0;
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

    if ( fread(hd, sizeof(SACHEAD), 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC header %s\n", name);
        fclose(strm);
        return NULL;
    }

    lswap = CheckSacHeaderVersion( hd->nvhdr );

    if ( lswap == -1 ) {
        fprintf(stderr, "Warning: %s not in sac format.\n", name);
        return NULL;
    } else if ( lswap == TRUE )
        ByteSwap( (char *)hd, HD_SIZE );
    
    sz = (size_t) hd->npts * sizeof(float);
    if ( (ar = (float *)malloc(sz) ) == NULL ) {
        fprintf(stderr, "Error in allocating memory for reading %s\n", name);
        return NULL;
    }

    if ( fread((char*)ar, sz, 1, strm) != 1 ) {
        fprintf(stderr, "Error in reading SAC data %s\n", name);
        return NULL;
    }
    fclose(strm);

    if ( lswap == TRUE ) ByteSwap( (char*) ar, sz);

    return ar;
}

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
