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

#include <stdlib.h>
#include <stdio.h>
#include <utime.h>
#include <unistd.h>
#include "external_compressors.h"

#include "iso9660.h"

int block_compress_file(FILE *input, FILE *output, off_t size)
{
  struct compressed_file_header hdr;
  compressor_data cmps;
  unsigned char inbuf[CBLOCK_SIZE], outbuf[2 * CBLOCK_SIZE];
  size_t bytes, pointer_bytes, nblocks, block;
  size_t cbytes;
  char *pointer_block, *curptr;
  off_t position;
  int i;
  int force_compress = opt.force;
  int err = EX_SOFTWARE;

  if ((sizeof hdr) & 3)
  {
    fputs("INTERNAL ERROR: header is not a multiple of 4\n", stderr);
    abort();
  }

  memset(&hdr, 0, sizeof hdr);
  memcpy(&hdr.magic, zisofs_magic, sizeof zisofs_magic);
  hdr.header_size = (sizeof hdr) >> 2;
  hdr.block_size = CBLOCK_SIZE_LG2;
  set_uint64_two_731(size, &hdr.uncompressed_len_low, &hdr.uncompressed_len_high);

  if (fwrite(&hdr, sizeof hdr, 1, output) != 1)
    return EX_CANTCREAT;

  nblocks = (size + CBLOCK_SIZE - 1) >> CBLOCK_SIZE_LG2;
  pointer_bytes = 4 * (nblocks + 1);
  pointer_block = xmalloc(pointer_bytes);
  memset(pointer_block, 0, pointer_bytes);

  if (fseek(output, pointer_bytes, SEEK_CUR) == -1)
  {
    err = EX_CANTCREAT;
    goto free_ptr_bail;
  }

  curptr = pointer_block;
  position = sizeof hdr + pointer_bytes;

  cmps.algorithm = opt.algorithm;

  init_compressor(&cmps);
  if (cmps.compressor == NULL)
  {
    err = EX_SOFTWARE;
    goto free_ptr_bail;
  }

  block = 0;
  while ((bytes = fread(inbuf, 1, CBLOCK_SIZE, input)) > 0)
  {
    if (bytes < CBLOCK_SIZE && block < nblocks - 1)
    {
      err = EX_IOERR;
      goto free_ptr_bail;
    }

    /* HACK: If the file has our magic number, always compress */
    if (block == 0 && bytes >= sizeof zisofs_magic)
    {
      if (!memcmp(inbuf, zisofs_magic, sizeof zisofs_magic))
        force_compress = 1;
    }

    set_731(curptr, position);
    curptr += 4;

    /* We have two special cases: a zero-length block is defined as all zero,
       and a block the length of which is equal to the block size is unencoded. */

    for (i = 0; i < (int)CBLOCK_SIZE; i++)
    {
      if (inbuf[i])
        break;
    }

    if (i == CBLOCK_SIZE)
    {
      /* All-zero block.  No output */
    }
    else
    {
      hdr.algorithm = opt.algorithm;
      cbytes = 2 * CBLOCK_SIZE;
      if ((err = compress_alg(&cmps, outbuf, &cbytes, inbuf, bytes, opt.level)) != EX_OK)
      {
        goto free_ptr_bail; /* Compression failure */
      }
      if (fwrite(outbuf, 1, cbytes, output) != cbytes)
      {
        err = EX_CANTCREAT;
        goto free_ptr_bail;
      }
      position += cbytes;
    }
    block++;
  }

  free_compressor(&cmps);

  /* Set pointer to the end of the final block */
  set_731(curptr, position);

  /* Now write the pointer table */
  if (fseek(output, sizeof hdr, SEEK_SET) == -1 ||
      fwrite(pointer_block, 1, pointer_bytes, output) != pointer_bytes)
  {
    err = EX_CANTCREAT;
    goto free_ptr_bail;
  }

  free(pointer_block);

  /* Now make sure that this was actually the right thing to do */
  if (!force_compress && position >= size)
  {
    /* Incompressible file, just copy it */
    rewind(input);
    rewind(output);

    position = 0;
    while ((bytes = fread(inbuf, 1, CBLOCK_SIZE, input)) > 0)
    {
      if (fwrite(inbuf, 1, bytes, output) != bytes)
        return EX_CANTCREAT;
      position += bytes;
    }

    /* Truncate the file to the correct size */
    fflush(output);
    ftruncate(fileno(output), position);
  }

  /* If we get here, we're done! */
  return 0;

  /* Common bailout code */
free_ptr_bail:
  free(pointer_block);
  return err;
}
