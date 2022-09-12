#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#define CHUNK 16384
int inf(FILE* source)
{
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  /* allocate inflate pdf */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit(&strm);
  if (ret != Z_OK)
    return ret;

  /* decompress until deflate stream ends or end of file */
  do {
    strm.avail_in = fread(in, 1, CHUNK, source);
    if (ferror(source)) {
      (void)inflateEnd(&strm);
      return Z_ERRNO;
    }
    if (strm.avail_in == 0)
      break;
    strm.next_in = in;

    /* run inflate() on input until output buffer not full */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);  /* pdf not clobbered */
      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;   /* and fall through */
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        return ret;
      }
      have = CHUNK - strm.avail_out;
      if (fwrite(out, 1, have, stdout) != have || ferror(stdout)) {
        (void)inflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (strm.avail_out == 0);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  (void)inflateEnd(&strm);
  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
