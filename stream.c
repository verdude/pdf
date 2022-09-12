#include <stdio.h>

#include "object.h"
#include "next.h"

stream_t* try_read_stream(state_t* state, long len) {
  char* stream_end = "endstream";
  char* stream_start = stream_end + 3;
  size_t match = check_for_match_seek_back(state->fs, stream_start);

  if (!match) {
    return NULL;
  }

  stream_t* stream = allocate(sizeof(stream_t));
  stream->bytes = fs_read(state->fs, len);
  // TODO: validate a single newline sequence.
  consume_whitespace(state->fs);
  match = check_for_match(state->fs, stream_end);
  if (!match) {
    fprintf(stderr, "Missing endstream.\n");
    cexit(state->fs, 1);
  }

  return stream;
}
