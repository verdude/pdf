#include <stdio.h>
#include <stdlib.h>

#include "xref.h"
#include "object.h"

xref_t* get_xref(FILE* fs, long offset) {
  return NULL;
}

void free_xref_t(xref_t* x) {
  for (int i = 0; i < x->count; ++i) {
    free(x->entries[i]);
  }
  free(x->entries);
  free(x);
}

