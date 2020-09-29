#ifndef EXTERNALCOMPRESSOR_H
#define EXTERNALCOMPRESSOR_H

#include "mkzftree.h"
#include "iso9660.h"

int compress_zlib(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len, int zlevel);
int uncompress_zlib(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len);

#endif /* MKZFTREE_H */
