#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "next.h"
#include "object.h"
#include "trailer.h"

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
  t->offset = get_pos(fs) + trailer_len;
  printf("Found trailer at: %li\n", t->offset);
  // TODO: make sure it is a dictionary...
  t->dictionary = next_sym(fs);

  print_list((list_t*) t->dictionary->val);

  return t;
}

int free_trailer_t(trailer_t* t) {
  int success = free_object_t(t->dictionary);
  free(t);
  return success;
}

