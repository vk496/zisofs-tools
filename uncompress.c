/* $Id$ */
/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2001-2006 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

#include "mkzftree.h" /* Must be included first! */

#include <stdio.h>
#include <stdlib.h>
#include "external_compressors.h"

#include "iso9660.h"

int block_uncompress_file(FILE *input, FILE *output, off_t size)
{
  struct compressed_file_header hdr;
  unsigned char *inbuf, *outbuf;
  int block_shift;
  char *pointer_block, *pptr;
  unsigned long nblocks;
  unsigned long block_size, block_size2;
  uint64_t fullsize;
  size_t ptrblock_bytes;
  unsigned long cstart, cend, csize;
  unsigned long bytes;
  int err = EX_SOFTWARE;

  if ((bytes = fread(&hdr, 1, sizeof hdr, input)) != sizeof hdr)
  {
    if (bytes == (size_t)size)
    {
      /* Very short file; not compressed */
      return (fwrite(&hdr, 1, bytes, output) != bytes) ? EX_CANTCREAT : 0;
    }
    else
    {
      return EX_IOERR; /* Read error */
    }
  }

  if (memcmp(&hdr.magic, zisofs_magic, sizeof zisofs_magic))
  {
    inbuf = xmalloc(CBLOCK_SIZE);
    /* Not compressed */
    memcpy(inbuf, &hdr, sizeof hdr);
    bytes = sizeof hdr;
    do
    {
      if (fwrite(inbuf, 1, bytes, output) != bytes)
        return EX_CANTCREAT;
    } while ((bytes = fread(inbuf, 1, CBLOCK_SIZE, input)) > 0);
    free(inbuf);
    return ferror(input) ? EX_IOERR : 0;
  }

  /* Now we know the file must be compressed.  Get the pointer table. */
  if (fseek(input, hdr.header_size << 2, SEEK_SET) == -1)
    return EX_IOERR;

  fullsize = get_uint64_two_731(&hdr.uncompressed_len_low, &hdr.uncompressed_len_high);
  block_shift = hdr.block_size;
  block_size = 1UL << block_shift;
  block_size2 = block_size << 1;
  inbuf = xmalloc(block_size2);
  outbuf = xmalloc(block_size);

  nblocks = (fullsize + block_size - 1) >> block_shift;

  ptrblock_bytes = (nblocks + 1) * 4;
  pointer_block = xmalloc(ptrblock_bytes);

  if ((bytes = fread(pointer_block, 1, ptrblock_bytes, input)) != ptrblock_bytes)
  {
    err = EX_IOERR;
    goto free_ptr_bail;
  }

  pptr = pointer_block;
  while (fullsize)
  {
    cstart = get_731(pptr);
    pptr += 4;
    cend = get_731(pptr);

    csize = cend - cstart;

    if (csize == 0)
    {
      memset(outbuf, 0, block_size);
      bytes = block_size;
    }
    else
    {
      if (csize > block_size2)
      {
        err = EX_DATAERR;
        goto free_ptr_bail;
      }

      if (fseek(input, cstart, SEEK_SET) == -1 ||
          (bytes = fread(inbuf, 1, csize, input)) != csize)
      {
        err = EX_IOERR;
        goto free_ptr_bail;
      }

      bytes = block_size; /* Max output buffer size */
      if ((err = uncompress_alg(hdr.algorithm, outbuf, &bytes, inbuf, csize)) != EX_OK)
      {
        goto free_ptr_bail;
      }
    }

    if (((fullsize > block_size) && (bytes != block_size)) || ((fullsize <= block_size) && (bytes < fullsize)))
    {
      err = EX_DATAERR;
      goto free_ptr_bail;
    }

    if (bytes > fullsize)
      bytes = fullsize;

    if (fwrite(outbuf, 1, bytes, output) != bytes)
    {
      err = EX_CANTCREAT;
      goto free_ptr_bail;
    }

    fullsize -= bytes;
  }

  err = 0;

free_ptr_bail:
  free(pointer_block);
  free(inbuf);
  free(outbuf);
  return err;
}
