#include <stdio.h>
#include <string.h>

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

  log_e("Invalid EOL Marker seq: 0x%02x 0x%02x", c, c2);
  return 0;
}

int dump_stream(stream_t* stream, char* fname) {
  FILE* f = fopen(fname, "wb");
  int written = fwrite(stream->bytes, 1, stream->len, f);

  if (written != stream->len) {
    log_v("oh no oh my");
  }
  log_v("writtine %i", written);
  fclose(f);
  return written;
}

static stream_t* read_stream(pdf_t* pdf, long len, char* stream_end) {
  stream_t* stream = allocate(sizeof(stream_t));
  int eol_len = read_eol_marker(pdf);
  if (!eol_len) {
    log_e("Bad eol after stream start");
    scexit(pdf, 1);
  }

  log_v("Reading %li bytes from start of stream at: %li", len, get_pos(pdf));
  stream->bytes = fs_read(pdf, len);
  stream->len = len;

  eol_len = read_eol_marker(pdf);
  if (!eol_len) {
    scexit(pdf, 1);
  }

  size_t match = check_for_match(pdf, stream_end);
  if (!match) {
    log_e("Missing endstream.");
    scexit(pdf, 1);
  }
  return stream;
}

stream_t* try_read_stream(pdf_t* pdf, long len) {
  log_v("attempting to get stream");
  char* stream_end = "endstream";
  char* stream_start = stream_end + 3;
  size_t match = check_for_match(pdf, stream_start);
  stream_t* stream = NULL;

  if (!match) {
    return NULL;
  }

  if (len <= 0) {
    log_i("skipping stream of length: %li at: %li", len, get_pos(pdf));
    find(pdf, stream_end, strlen(stream_end), FORWARD);
  }
  else {
    stream = read_stream(pdf, len, stream_end);
  }

  consume_whitespace(pdf);
  return stream;
}
