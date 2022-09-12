#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "next.h"
#include "object.h"
#include "trailer.h"
#include "pdf.h"

object_t* get_encryption(trailer_t* trailer) {
  return get_entry_value(trailer->dictionary, "Encrypt");
}

int get_trailer(pdf_t* pdf) {
  seek(pdf->fs, -(EOF_LEN+1), SEEK_END);
  trailer_t* t = allocate(sizeof(trailer_t));

  char* startxref_string = "\nstartxref\n";
  size_t startxref_len = strlen(startxref_string);

  int xro_offset = find_backwards(pdf->fs, startxref_string, startxref_len + 1);
  if (xro_offset) {
    long xro = get_num(pdf, 0, FAIL);
    t->startxref_offset = xro;
  } else {
    fprintf(stderr, "startxref string not found.\n");
  }

  char* trailer_string = "\ntrailer\n";
  size_t trailer_len = strlen(trailer_string);
  int found = find_backwards(pdf->fs, trailer_string, trailer_len + 1);

  if (!found) {
    fprintf(stderr, "Failed to find trailer.\n");
    free(t);
    return 0;
  }

  t->offset = get_pos(pdf->fs) - trailer_len;

  // TODO: make sure it is a dictionary...
  t->dictionary = next_sym(pdf);

  t->encryption = get_encryption(t);
  if (!t->encryption) {
    fprintf(stderr, "Warning: Encryption key not found.\n");
  } else {
    printf("encryption:\n");
    print_object(t->encryption);
  }

  printf("startxref: %li\n", t->startxref_offset);

  pdf->trailer = t;
  return 1;
}

void free_trailer_t(trailer_t* t) {
  if (t) {
    if (t->dictionary) {
      free_object_t(t->dictionary);
    }
    free(t);
  }
}

