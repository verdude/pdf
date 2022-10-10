#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "next.h"
#include "object.h"
#include "trailer.h"
#include "pdf.h"
#include "log.h"

object_t* get_encryption(trailer_t* trailer) {
  return get_entry_value(trailer->dictionary, "Encrypt");
}

int get_trailer(pdf_t* pdf) {
  seek(pdf, -(EOF_LEN+1), SEEK_END);
  trailer_t* t = allocate(sizeof(trailer_t));

  char* startxref_string = "\nstartxref\n";
  size_t startxref_len = strlen(startxref_string);

  int xro_offset = find(pdf, startxref_string, startxref_len + 1, BACKWARD);
  if (xro_offset) {
    long xro = get_num(pdf, 0, FAIL);
    t->startxref_offset = xro;
  } else {
    log_e("startxref string not found.");
  }

  char* trailer_string = "\ntrailer";
  size_t trailer_len = strlen(trailer_string);
  int found = find(pdf, trailer_string, trailer_len + 1, BACKWARD);

  if (!found) {
    log_e("Failed to find trailer.");
    free(t);
    return 0;
  }

  t->offset = get_pos(pdf) - trailer_len;

  // TODO: make sure it is a dictionary...
  t->dictionary = next_sym(pdf);

  t->encryption = get_encryption(t);
  if (!t->encryption) {
    log_e("Warning: Encryption key not found.");
  } else {
    log_v("encryption:");
    print_object(t->encryption);
  }

  log_v("startxref: %li", t->startxref_offset);

  pdf->trailer = t;
  print_object(pdf->trailer->dictionary);
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

