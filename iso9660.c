/* $Id$ */
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

#include "iso9660.h"

/* zisofs magic */

const unsigned char zisofs_magic[8] = {
    0x37, 0xE4, 0x53, 0x96, 0xC9, 0xDB, 0xD6, 0x07};

/* iso9660 integer formats */

void set_721(void *pnt, unsigned int i)
{
  unsigned char *p = (unsigned char *)pnt;
  p[0] = i & 0xff;
  p[1] = (i >> 8) & 0xff;
}

unsigned int
get_721(void *pnt)
{
  unsigned char *p = (unsigned char *)pnt;
  return ((unsigned int)p[0]) + ((unsigned int)p[1] << 8);
}

void set_722(void *pnt, unsigned int i)
{
  unsigned char *p = (unsigned char *)pnt;
  p[0] = (i >> 8) & 0xff;
  p[1] = i & 0xff;
}

unsigned int
get_722(void *pnt)
{
  unsigned char *p = (unsigned char *)pnt;
  return ((unsigned int)p[0] << 8) + ((unsigned int)p[1]);
}

void set_723(void *pnt, unsigned int i)
{
  unsigned char *p = (unsigned char *)pnt;
  p[3] = p[0] = i & 0xff;
  p[2] = p[1] = (i >> 8) & 0xff;
}

#define get_723(x) get_721(x)

void set_731(void *pnt, unsigned int i)
{
  unsigned char *p = (unsigned char *)pnt;
  p[0] = i & 0xff;
  p[1] = (i >> 8) & 0xff;
  p[2] = (i >> 16) & 0xff;
  p[3] = (i >> 24) & 0xff;
}

unsigned int
get_731(void *pnt)
{
  unsigned char *p = (unsigned char *)pnt;
  return ((unsigned int)p[0]) + ((unsigned int)p[1] << 8) +
         ((unsigned int)p[2] << 16) + ((unsigned int)p[3] << 24);
}

void set_732(void *pnt, unsigned int i)
{
  unsigned char *p = (unsigned char *)pnt;
  p[3] = i & 0xff;
  p[2] = (i >> 8) & 0xff;
  p[1] = (i >> 16) & 0xff;
  p[0] = (i >> 24) & 0xff;
}

unsigned int
get_732(void *pnt)
{
  unsigned char *p = (unsigned char *)pnt;
  return ((unsigned int)p[0] << 24) + ((unsigned int)p[1] << 16) +
         ((unsigned int)p[2] << 8) + ((unsigned int)p[3]);
}

void set_733(void *pnt, unsigned int i)
{
  unsigned char *p = (unsigned char *)pnt;
  p[7] = p[0] = i & 0xff;
  p[6] = p[1] = (i >> 8) & 0xff;
  p[5] = p[2] = (i >> 16) & 0xff;
  p[4] = p[3] = (i >> 24) & 0xff;
}

void set_uint64_two_731(uint64_t size, void* ufsl, void* ufsh) {
  uint32_t ul = (uint32_t) (size & 0xffffffff);
  uint32_t uh = (uint32_t) (size >> 32);

  set_731(ufsl, ul);
  set_731(ufsh, uh);
}

uint64_t get_uint64_two_731(void* ufsl, void* ufsh) {
  uint32_t ul = get_731(ufsl);
  uint32_t uh = get_731(ufsh);

  return (uint64_t) (((uint64_t)uh << 32) | ((uint64_t) ul & 0xffffffff));
}

#define get_733(x) get_731(x)
