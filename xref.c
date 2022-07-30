#include <stdio.h>
#include <stdlib.h>

#include "xref.h"
#include "next.h"
#include "object.h"

void print_x_entry(x_entry_t* e) {
    printf("%li %li %c\n", e->offset, e->gen, e->status);
}

void print_xref(xref_t* x) {
  printf("%li %li\n", x->obj_num, x->count);
  printf("INDEX: %li\n", x->index);
}

void free_xref_t(xref_t* x) {
  free(x);
}

static int read_size(FILE* fs, xref_t* x) {
  x->obj_num = get_num(fs);
  consume_whitespace(fs);

  x->count = get_num(fs);
  consume_whitespace(fs);

  return 1;
}

// TODO: read space/2 char eol instead of any whitespace
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

  xref->index = get_pos(fs) + ENTRY_WIDTH;

  return xref;
}

