
#include <zlib.h>
#include <string.h>

#include "external_compressors.h"

int zlib_compress(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len, int zlevel);
int zlib_uncompress(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len);
int lzma_compress(compressor_data *cmps, uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len);

// Public functions

int compress_alg(compressor_data *cmps, uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len, int zlevel)
{
  /* IMPORTANT
    outbuf_len MUST be updated with the compressed size of the buffer
  */

  switch (cmps->algorithm)
  {
  case ZISOFS_ZLIB:
    return zlib_compress(outbuf, outbuf_len, inbuf, inbuf_len, zlevel);
  case ZISOFS_LZMA:
    return lzma_compress(cmps, outbuf, outbuf_len, inbuf, inbuf_len);
  default:
    return EX_SOFTWARE;
  }
}

int uncompress_alg(compressor_data *cmps, uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len)
{
  // uncompress(outbuf, &bytes, inbuf, csize);
  switch (cmps->algorithm)
  {
  case ZISOFS_ZLIB:
    return zlib_uncompress(outbuf, outbuf_len, inbuf, inbuf_len);
  default:
    return EX_SOFTWARE;
  }
}

void init_compressor(compressor_data *cmps)
{

  lzma_ret ret;

  switch (cmps->algorithm)
  {
  case ZISOFS_ZLIB:
    cmps->compressor = malloc(1); //ignore
    break;
  case ZISOFS_LZMA:;

    lzma_stream *strm = calloc(1, sizeof(*strm));
    ret = lzma_easy_encoder(strm, 6, LZMA_CHECK_CRC64);

    if (ret != LZMA_OK)
    {
      //TODO: Print debug info of error here
      cmps->compressor = NULL;
    }

    cmps->compressor = strm;

    break;
  default:
    break;
  }
}

void free_compressor(compressor_data *cmps)
{
  switch (cmps->algorithm)
  {
  case ZISOFS_ZLIB:
    free(cmps->compressor);
    break;
  case ZISOFS_LZMA:
    lzma_end(cmps->compressor);
  default:
    break;
  }
}

/* -------------------------------------------------------------- */

// ZLIB
int zlib_compress(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len, int zlevel)
{
  int ex_code;

  if ((ex_code = compress2((Bytef *)outbuf, (uLongf *)outbuf_len, (Bytef *)inbuf, (uLong)inbuf_len, zlevel)) != Z_OK)
  {
    return (ex_code == Z_MEM_ERROR) ? EX_OSERR : EX_SOFTWARE;
  }
  else
  {
    return EX_OK;
  }
}

int zlib_uncompress(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len)
{
  int ex_code;

  if ((ex_code = uncompress((Bytef *)outbuf, (uLongf *)outbuf_len, (Bytef *)inbuf, (uLong)inbuf_len)) != Z_OK)
  {
    return (ex_code == Z_MEM_ERROR) ? EX_OSERR : EX_DATAERR;
  }
  else
  {
    return EX_OK;
  }
}

// LZMA

int lzma_compress(compressor_data *cmps, uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len)
{

  cmps->compressor->next_in = inbuf;
  cmps->compressor->avail_in = inbuf_len;
  cmps->compressor->next_out = outbuf;
  cmps->compressor->avail_out = *outbuf_len;

  lzma_ret ret = lzma_code((cmps->compressor), LZMA_RUN);

  if (ret != LZMA_OK || cmps->compressor->avail_in != 0) //TODO: Review if bufin will be always readed completly
  {
    return EX_SOFTWARE;
  }
  else
  {
    *outbuf_len = *outbuf_len - cmps->compressor->avail_out;
    return EX_OK;
  }
}
