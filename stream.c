#include <stdio.h>

#include "object.h"
#include "next.h"
#include "log.h"

int read_eol_marker(pdf_t* pdf) {
  int c = get_char(pdf, FAIL);
  int c2 = 0;
  if (c == '\n') {
    return 1;
  }

  if (c == '\r') {
    c2 = get_char(pdf, FAIL);
    if (c2 == '\n') {
      return 2;
    }
  }

  fprintf(stderr, "Invalid EOL Marker seq: 0x%02x 0x%02x\n", c, c2);
  return 0;
}

int dump_stream(stream_t* stream, char* fname) {
  FILE* f = fopen(fname, "wb");
  int written = fwrite(stream->bytes, 1, stream->len, f);

  if (written != stream->len) {
    printf("oh no oh my\n");
  }
  printf("writtine %i\n", written);
  fclose(f);
  return written;
}

stream_t* try_read_stream(pdf_t* pdf, long len) {
  printf("attempting to get stream\n");
  char* stream_end = "endstream";
  char* stream_start = stream_end + 3;
  size_t match = check_for_match(pdf, stream_start);

  if (!match) {
    return NULL;
  }

  stream_t* stream = allocate(sizeof(stream_t));
  int eol_len = read_eol_marker(pdf);
  if (!eol_len) {
    fprintf(stderr, "Bad eol after stream start\n");
    scexit(pdf, 1);
  }
  printf("Reading %li bytes from start of stream at: %li\n", len, get_pos(pdf));
  stream->bytes = fs_read(pdf, len);
  stream->len = len;

  eol_len = read_eol_marker(pdf);
  if (!eol_len) {
    scexit(pdf, 1);
  }

  match = check_for_match(pdf, stream_end);
  if (!match) {
    fprintf(stderr, "Missing endstream.\n");
    scexit(pdf, 1);
  }

  scexit(pdf, 0);
  consume_whitespace(pdf);
  return stream;
}
