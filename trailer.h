#ifndef tailer_h
#define trailer_h

#define EOF_LEN 5

typedef struct {
  long offset;
} trailer_t;

trailer_t* get_trailer();

#endif // trailer_h