#ifndef tailer_h
#define trailer_h

#define EOF_LEN 5

#include "object.h"

typedef struct {
  long offset;
  object_t* dictionary;
} trailer_t;

trailer_t* get_trailer();

int free_trailer_t(trailer_t* t);

#endif // trailer_h
