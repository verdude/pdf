#include <stdio.h>
#include <stdlib.h>

#include "pdf.h"
#include "xref.h"
#include "next.h"
#include "object.h"
#include "log.h"

void print_x_entry(x_entry_t* e) {
    printf("%li %li %c\n", e->offset, e->gen, e->status);
}

void print_xref(xref_t* x) {
  printf("%li %li\n", x->obj_num, x->count);
  printf("Current entry offset: %li\n", x->ce_offset);
  printf("Current entry index: %li\n", x->ce_index);
}

void free_xref_t(xref_t* x) {
  if (x) {
    free(x);
  }
}

static int read_size(pdf_t* pdf) {
  pdf->xref->obj_num = get_num(pdf, 0, FAIL);
  consume_whitespace(pdf);

  pdf->xref->count = get_num(pdf, 0, FAIL);
  consume_whitespace(pdf);

  return 1;
}

static long get_nth_offset(pdf_t* pdf, long n) {
  long offset_in_table = n * ENTRY_WIDTH;
  return pdf->xref->t_offset + offset_in_table;
}

static void checkout_next_obj(pdf_t* pdf) {
  xref_t* xref = pdf->xref;
  if (xref->ce_index + 1 >= xref->count) {
    printf("attempt to seek past end of xref table. Entry %li\n",
        xref->ce_index);
    return;
  }

  xref->ce_offset = xref->ce_offset + ENTRY_WIDTH;
  xref->ce_index++;
  seek(pdf, xref->ce_offset, SEEK_SET);
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
static int get_status(pdf_t* pdf) {
  if (!valid_xref_entry(pdf->xref)) {
    log_e("Invalid xref entry offset %li\n", pdf->xref->ce_offset);
    return -1;
  }

  seek(pdf, ENTRY_WIDTH - 3, SEEK_CUR);
  int c = get_char(pdf, FAIL);
  switch ((unsigned char) c) {
    case 'f':
      return 0;
    case 'n':
      return 1;
    default:
      log_e("Invalid xref entry char: [%c].\n", c);
      return -1;
  }
}

/**
 * Returns the offset of the object as listed in the current
 * entry.
 */
static long get_obj_offset(pdf_t* pdf) {
  seek(pdf, pdf->xref->ce_offset, SEEK_SET);
  return get_num(pdf, 10, FAIL);
}

object_t* next_obj(pdf_t* pdf) {
  printf("Getting xref entry #%li\n", pdf->xref->ce_index);
  int status;
  seek(pdf, pdf->xref->ce_offset, SEEK_SET);
  while (!(status = get_status(pdf))) {
    if (status == -1) {
      log_e("Invalid entry status.");
      scexit(pdf, 1);
    }
    printf("Skipping entry %li with invalid status: %i\n", pdf->xref->ce_offset, status);
    checkout_next_obj(pdf);
  }

  long offset = get_obj_offset(pdf);

  seek(pdf, offset, SEEK_SET);

  printf("Getting next obj at %li\n", get_pos(pdf));
  return next_sym(pdf);
}

object_t* get_object(pdf_t* pdf, int obj_num) {
  xref_t* x = pdf->xref;
  int i = x->obj_num;
  if (obj_num < i || obj_num > x->obj_num + x->count) {
    log_e("Could not find obj #%i in the xref table\n", obj_num);
    scexit(pdf, 1);
  }
  pdf->xref->ce_offset = get_nth_offset(pdf, obj_num);
  pdf->xref->ce_index = obj_num;
  int status = get_status(pdf);
  if (status == -1 || !status) {
    log_e("Tried to read object with invalid status: %i\n", status);
    scexit(pdf, 1);
  }
  return next_obj(pdf);
}

int get_xref(pdf_t* pdf) {
  char* xref_string = "xref";
  size_t match;
  long offset = pdf->trailer->startxref_offset;

  seek(pdf, offset, SEEK_SET);
  if (!(match = check_for_match(pdf, xref_string))) {
    log_e("Did not find xref table at offset: %li\n", offset);
    return 0;
  }

  consume_whitespace(pdf);
  pdf->xref = allocate(sizeof(xref_t));

  int success = read_size(pdf);
  if (!success) {
    log_e("Failed to read xref obj index/size.\n");
    free_xref_t(pdf->xref);
    return 0;
  }

  pdf->xref->t_offset = get_pos(pdf);
  pdf->xref->ce_offset = pdf->xref->t_offset;
  pdf->xref->ce_index = 0;

  return 1;
}

static int has_next(xref_t* xref) {
  return xref->ce_index < xref->count - 1;
}

void parse_entries(pdf_t* pdf) {
  for (int i = pdf->xref->ce_index; i < pdf->xref->count; ++i) {
    if (!has_next(pdf->xref)) {
      printf("Traversed whole xref table.\n");
      break;
    }
    object_t* o = next_obj(pdf);
    printf("Type Name: %s\n", get_type_name(o));
    free_object_t(o);
    checkout_next_obj(pdf);
  }
}
