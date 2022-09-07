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
  printf("Current entry offset: %li\n", x->ce_offset);
  printf("Current entry index: %li\n", x->ce_index);
}

void free_xref_t(xref_t* x) {
  free(x);
}

static int read_size(FILE* fs, xref_t* x) {
  x->obj_num = get_num(fs, 0, FAIL);
  consume_whitespace(fs);

  x->count = get_num(fs, 0, FAIL);
  consume_whitespace(fs);

  return 1;
}

static long get_nth_offset(xref_t* xref, long n) {
  long offset_in_table = n * ENTRY_WIDTH;
  return xref->t_offset + offset_in_table;
}

static void checkout_next_obj(FILE* fs, xref_t* xref) {
  if (xref->ce_index + 1 >= xref->count) {
    printf("attempt to seek past end of xref table. Entry %li\n",
        xref->ce_index);
    return;
  }

  xref->ce_offset = xref->ce_offset + ENTRY_WIDTH;
  xref->ce_index++;
  seek(fs, xref->ce_offset, SEEK_SET);
}

// returns 1 if pointing to a valid xref entry
static int valid_xref_entry(xref_t* xref) {
  int total_bytes = xref->count * ENTRY_WIDTH,
      curr_byte = xref->ce_index * ENTRY_WIDTH,
      largest_valid_index = total_bytes - ENTRY_WIDTH;

  return curr_byte <= largest_valid_index;
}

/**
 * Returns:
 * -1 for invalid entry status
 * 1 for normal entry
 * 0 for free entry
 */
static int get_status(FILE* fs, xref_t* xref) {
  if (!valid_xref_entry(xref)) {
    fprintf(stderr, "Invalid xref entry offset: %li\n", xref->ce_offset);
    return -1;
  }

  seek(fs, ENTRY_WIDTH - 3, SEEK_CUR);
  int c = get_char(fs, FAIL);
  switch ((unsigned char) c) {
    case 'f':
      return 0;
    case 'n':
      return 1;
    default:
      fprintf(stderr, "Invalid xref entry char: [%c].\n", c);
      return -1;
  }
}

/**
 * Returns the offset of the object as listed in the current
 * entry.
 */
static long get_obj_offset(FILE* fs, xref_t* xref) {
  seek(fs, xref->ce_offset, SEEK_SET);
  return get_num(fs, 10, FAIL);
}

object_t* next_obj(FILE* fs, xref_t* xref) {
  int status;
  seek(fs, xref->ce_offset, SEEK_SET);
  while (!(status = get_status(fs, xref))) {
    if (status == -1) {
      fprintf(stderr, "Invalid entry status.");
      cexit(fs, 1);
      return NULL;
    }
    checkout_next_obj(fs, xref);
  }

  long offset = get_obj_offset(fs, xref);

  seek(fs, offset, SEEK_SET);

  return next_sym(fs);
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

  xref->t_offset = get_pos(fs);
  xref->ce_offset = xref->t_offset;
  xref->ce_index = 0;

  return xref;
}

static int has_next(xref_t* xref) {
  return xref->ce_index < xref->count - 1;
}

void parse_entries(FILE* fs, xref_t* xref) {
  for (int i = xref->ce_index; i < xref->count; ++i) {
    if (!has_next(xref)) {
      printf("Traversed whole xref table.\n");
      break;
    }
    object_t* o = next_obj(fs, xref);
    free_object_t(o);
    checkout_next_obj(fs, xref);
  }
}

