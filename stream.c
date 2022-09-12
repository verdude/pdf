#include <stdio.h>

#include "object.h"
#include "next.h"

stream_t* try_read_stream(pdf_t* pdf, long len) {
  char* stream_end = "endstream";
  char* stream_start = stream_end + 3;
  size_t match = check_for_match_seek_back(pdf->fs, stream_start);

  if (!match) {
    return NULL;
  }

  stream_t* stream = allocate(sizeof(stream_t));
  stream->bytes = fs_read(pdf->fs, len);
  // TODO: validate a single newline sequence.
  consume_whitespace(pdf->fs);
  match = check_for_match(pdf->fs, stream_end);
  if (!match) {
    fprintf(stderr, "Missing endstream.\n");
    cexit(pdf->fs, 1);
  }

  return stream;
}
