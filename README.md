# SAC Tools

## SAC I/O functions

- `sacio.h`: Head file for SAC file format, and prototype for SAC IO functions.
- `sacio.c`: Definitions of several SAC IO functions.
  - `read_sac_head`: read SAC header
  - `read_sac`: read SAC binary data
  - `read_sac_xy`: read SAC binary XY data
  - `read_sac_pdw`: read SAC data in a partial data window (cut option)
  - `write_sac`: write SAC binary data
  - `write_sac_xy`: write SAC binary XY data
  - `new_sac_head`: create a minimal SAC header
  - `sac_head_index`: return the index of a SAC head field
  - `issac`: Check if a file in in SAC format

## SAC Utilities

- [sac2col](#sac2col): Convert a SAC file to a one/two column table.
- [saclh](#saclh): List the values of selected head fields.
- [sacch](#sacch): Change the value of selected head fields.
- [sacmax](#sacmax): Get max amplitude of SAC files in a specified time window.

### `sac2col`


```
Convert a SAC file to a one/two column table.

Usage:
  sac2col [-C <cols>] [-h] sacifle

Options:
  -C <cols>    output data in 1 or 2 column.
  -h           show usage.
```

### `saclh`

```
List the values of selected head fields

Usage:
  saclh -H head_fields_list [-N] sacfiles

Options:
  -H: list of SAC head fields
  -N: do not output filename in 1st colunm

Note:
  1. SAC head fields should be seperated by commas.

Examples:
  saclh -H evla,evlo,stla,stlo seis1 seis2
  saclh -H evla -N seis
```

### `sacch`

```
Change the value of selected head fields

Usgae:
   sacch key1=value1 key2=value2 ... sacfiles
   sacch time=DATETIME sacfiles
   sacch allt=value sacfiles

Notes:
   1. keys are sac head fields, like npts, evla
   2. values are integers, floats or strings
   3. key=undef to set key to undefinded value.
   4. DATETIME format: yyyy-mm-ddThh:mm:ss.mmm
   5. variables for time offset can use value
      in DATETIME format
   6. allt: add seconds to all defined header
      times, and subtract seconds from refer time

Examples:
   sacch stla=10.2 stlo=20.2 kstnm=COLA seis1 seis2
   sacch time=2010-02-03T10:20:35.200 seis1 seis2
   sacch t7=2010-02-03T10:20:30.000 seis1
   sacch t9=undef kt9=undef seis*
   sacch allt=10.23 seis*
```

### `sacmax`

```
Get max amplitude of SAC files in a specified time window.

Usage:
  sacmax -Mmode [-Ttmark/t0/t1] sacfiles

  Options:
    -M0   return maximum amplitude
    -M1   return minumum amplitude
    -M2   return maximum absolute amplitude
    -M3   return absolute maximum amplitude
    -M4   return maximum peak-to-peak amplitude
    -T    specify time window.
    -h    show usage.
```

Examples:
   sacmax -M0 -T0/5/10 seis1
