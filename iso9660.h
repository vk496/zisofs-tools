#ident "$Id$"
/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2001 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/* zisofs definitions */

#ifndef ISO9660_H
#define ISO9660_H

#include <stdint.h>

#ifndef CBLOCK_SIZE_LG2
#define CBLOCK_SIZE_LG2 15 /* Compressed block size */
#endif
#define CBLOCK_SIZE (1 << CBLOCK_SIZE_LG2)

/* Compressed file magic */
extern const unsigned char zisofs_magic[8];

/* VERY VERY VERY IMPORTANT: Must be a multiple of 4 bytes */
struct compressed_file_header
{
  char magic[8];
  char uncompressed_len_low[4];
  unsigned char header_size;
  unsigned char block_size;
  uint8_t algorithm;
  char reserved[1];                 /* Reserved for future use, MBZ */
  uint8_t uncompressed_len_high[4]; // Used as extension of the size to handle files bigger than 2^32 bytes. unc_len2 << 32 | unc_len
  uint8_t reserved2[12];
};

typedef enum
{
  ZISOFS_ZLIB = 0, //zlib
  ZISOFS_LZMA = 1, //lzma
} zisofs_alg;


/* iso9660 integer formats */
void set_721(void *, unsigned int);
unsigned int get_721(void *);
void set_722(void *, unsigned int);
unsigned int get_722(void *);
void set_723(void *, unsigned int);
void set_731(void *, unsigned int);
unsigned int get_731(void *);
void set_732(void *, unsigned int);
unsigned int get_732(void *);
void set_733(void *, unsigned int);
#define get_723(x) get_721(x)
#define get_733(x) get_731(x)

/* Wrap uint64 in two ints */

void set_uint64_two_731(uint64_t size, void *ufsl, void *ufsh);
uint64_t get_uint64_two_731(void *ufsl, void *ufsh);

#endif /* ISO9660_H */
