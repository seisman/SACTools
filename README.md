SAC Tools
=========

This is a collection of C codes relating with SAC files.

- `sacio.h`: Head file for SAC file format, and prototype for SAC IO functions.
- `sacio.c`: Definitions of several SAC IO functions.
  - read_sac_head: read SAC header
  - read_sac: read SAC binary data
  - read_sac_xy: read SAC binary XY data
  - read_sac_pdw: read SAC data in a partial data window (cut option)
  - write_sac: write SAC binary data
  - write_sac_xy: write SAC binary XY data
  - new_sac_head: create a minimal SAC header

- sac2col: Convert a SAC file to a one/two column table
