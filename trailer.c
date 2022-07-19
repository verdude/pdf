#include <stdlib.h>
#include <stdio.h>

#include "trailer.h"
#include "next.h"

trailer_t* get_trailer(FILE* fs) {
  trailer_t* t = calloc(sizeof(trailer_t), 1);
  seek(fs, -(EOF_LEN+1), SEEK_END, FAIL);

  unsigned char* trailer_string = "\ntrailer\n";
  int pos = find_backwards(fs, trailer_string, 10);
  printf("Found trailer at: %i\n", pos);

  return t;
}
