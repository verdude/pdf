#include <stdio.h>
#include <stdlib.h>

#include "xref.h"
#include "next.h"
#include "object.h"

void free_xref_t(xref_t* x) {
  for (int i = 0; i < x->count; ++i) {
    free(x->entries[i]);
  }
  free(x->entries);
  free(x);
}

static int read_size(FILE* fs, xref_t* x) {
  x->obj_num = get_num(fs);
  consume_whitespace(fs);

  x->count = get_num(fs);
  consume_whitespace(fs);

  return 1;
}

static x_entry_t* read_entry(FILE* fs) {
  x_entry_t* e = allocate(sizeof(x_entry_t));

  e->offset = get_num(fs);
  consume_whitespace(fs);

  e->gen = get_num(fs);
  consume_whitespace(fs);

  e->status = (char) get_char(fs, FAIL);
  consume_whitespace(fs);

  return e;
}

static int read_entries(FILE* fs, xref_t* xref) {
  xref->entries = allocate(sizeof(x_entry_t*) * xref->count);
  for (int i = 0; i < xref->count; ++i) {
    xref->entries[0] = read_entry(fs);
  }
  return 1;
}

xref_t* get_xref(FILE* fs, long offset) {
  char* xref_string = "xref";
  xref_t* xref;
  size_t match;

  seek(fs, offset, SEEK_SET);
  if (!(match = check_for_match(fs, xref_string))) {
    fprintf(stderr, "Did not find xref table at offset: %li\n", offset);
    return NULL;
  }

  consume_whitespace(fs);
  xref = allocate(sizeof(xref_t));

  int success = read_size(fs, xref);
  if (!success) {
    fprintf(stderr, "Failed to read xref obj index/size.\n");
    free_xref_t(xref);
    return NULL;
  }

  success = read_entries(fs, xref);
  if (!success) {
    fprintf(stderr, "Failed to read xref table.\n");
    free_xref_t(xref);
    return NULL;
  }

  return xref;
}

