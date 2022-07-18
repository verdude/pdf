#include <stdlib.h>
#include <stdio.h>

#include "trailer.h"
#include "next.h"

trailer_t* get_trailer(FILE* fs) {
  trailer_t* t = calloc(sizeof(trailer_t), 1);
  int offset = seek(fs, -EOF_LEN, SEEK_END, FAIL);

  printf("offset: %i, char: %c", offset, (char) get_char(fs, FAIL));

  return t;
}
