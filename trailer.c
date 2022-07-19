#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trailer.h"
#include "next.h"

trailer_t* get_trailer(FILE* fs) {
  seek(fs, -(EOF_LEN+1), SEEK_END, FAIL);

  char* trailer_string = "\ntrailer\n";
  size_t trailer_len = strlen(trailer_string);
  int found = find_backwards(fs, trailer_string, trailer_len + 1);

  if (!found) {
    fprintf(stderr, "Failed to find trailer.\n");
    return NULL;
  }

  trailer_t* t = calloc(sizeof(trailer_t), 1);
  t->offset = get_pos(fs);
  printf("Found trailer at: %li\n", t->offset);

  return t;
}
