#ifndef tailer_h
#define trailer_h

#define EOF_LEN 5

#include "object.h"

typedef struct {
  long offset;
  object_t* dictionary;
  long startxref_offset;
  object_t* encryption;
} trailer_t;

trailer_t* get_trailer();

void free_trailer_t(trailer_t* t);

#endif // trailer_h
