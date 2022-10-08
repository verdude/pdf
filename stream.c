#include <stdio.h>

#include "object.h"
#include "next.h"

int read_eol_marker(pdf_t* pdf) {
  int c = get_char(pdf->fs, FAIL);
  int c2 = 0;
  if (c == '\n') {
    return 1;
  }

  if (c == '\r') {
    c2 = get_char(pdf->fs, FAIL);
    if (c2 == '\n') {
      return 2;
    }
  }

  fprintf(stderr, "Invalid EOL Marker seq: 0x%02x 0x%02x\n", c, c2);
  return 0;
}

stream_t* try_read_stream(pdf_t* pdf, long len) {
  printf("attempting to get stream\n");
  char* stream_end = "endstream";
  char* stream_start = stream_end + 3;
  size_t match = check_for_match(pdf->fs, stream_start);

  if (!match) {
    return NULL;
  }

  stream_t* stream = allocate(sizeof(stream_t));
  printf("Reading %li bytes from start of stream at: %li\n", len, get_pos(pdf->fs));
  stream->bytes = fs_read(pdf->fs, len);

  int eol_len = read_eol_marker(pdf);
  if (!eol_len) {
    scexit(pdf, 1);
  }

  printf("valid EOL for stream of len: %i\n", eol_len);
  match = check_for_match_seek_back(pdf->fs, stream_end);
  if (!match) {
    fprintf(stderr, "Missing endstream.\n");
    scexit(pdf, 1);
  }

  return stream;
}
