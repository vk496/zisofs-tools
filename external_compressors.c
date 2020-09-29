
#include <zlib.h>

#include "external_compressors.h"

int ex_code;

int compress_zlib(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len, int zlevel)
{

  if ((ex_code = compress2((Bytef *)outbuf, (uLongf *)outbuf_len, (Bytef *)inbuf, (uLong)inbuf_len, zlevel)) != Z_OK)
  {
    ex_code = (ex_code == Z_MEM_ERROR) ? EX_OSERR : EX_SOFTWARE;
  }
  else
  {
    ex_code = EX_OK;
  }

  return ex_code;
}

int uncompress_zlib(uint8_t *outbuf, size_t *outbuf_len, uint8_t *inbuf, size_t inbuf_len)
{
  // uncompress(outbuf, &bytes, inbuf, csize);

  if ((ex_code = uncompress((Bytef *)outbuf, (uLongf *)outbuf_len, (Bytef *)inbuf, (uLong)inbuf_len)) != Z_OK)
  {
    ex_code = (ex_code == Z_MEM_ERROR) ? EX_OSERR : EX_DATAERR;
  }
  else
  {
    ex_code = EX_OK;
  }

  return ex_code;
}
