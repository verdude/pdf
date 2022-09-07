#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "next.h"
#include "object.h"
#include "trailer.h"

object_t* get_encryption(trailer_t* trailer) {
  return get_entry_value(trailer->dictionary, "Encrypt");
}

trailer_t* get_trailer(FILE* fs) {
  seek(fs, -(EOF_LEN+1), SEEK_END);
  trailer_t* t = allocate(sizeof(trailer_t));

  char* startxref_string = "\nstartxref\n";
  size_t startxref_len = strlen(startxref_string);

  int xro_offset = find_backwards(fs, startxref_string, startxref_len + 1);
  if (xro_offset) {
    long xro = get_num(fs, 0, FAIL);
    t->startxref_offset = xro;
  } else {
    fprintf(stderr, "startxref string not found.\n");
  }

  char* trailer_string = "\ntrailer\n";
  size_t trailer_len = strlen(trailer_string);
  int found = find_backwards(fs, trailer_string, trailer_len + 1);

  if (!found) {
    fprintf(stderr, "Failed to find trailer.\n");
    free(t);
    return NULL;
  }

  t->offset = get_pos(fs) - trailer_len;

  // TODO: make sure it is a dictionary...
  t->dictionary = next_sym(fs);

  t->encryption = get_encryption(t);
  if (!t->encryption) {
    fprintf(stderr, "Warning: Encryption key not found.\n");
  } else {
    printf("encryption:\n");
    print_object(t->encryption);
  }

  printf("startxref: %li\n", t->startxref_offset);

  return t;
}

void free_trailer_t(trailer_t* t) {
  free_object_t(t->dictionary);
  free(t);
}

