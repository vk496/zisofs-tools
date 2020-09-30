#ifndef EXTERNALCOMPRESSOR_H
#define EXTERNALCOMPRESSOR_H

#include <lzma.h>
#include "mkzftree.h"
#include "iso9660.h"


typedef struct
{
  zisofs_alg algorithm;
  lzma_stream* compressor;
} compressor_data;

void init_compressor(compressor_data* cmps);
void free_compressor(compressor_data* cmps);


int compress_alg(compressor_data* algorithm, uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len, int zlevel);
int uncompress_alg(compressor_data* algorithm, uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len);

#endif /* MKZFTREE_H */
