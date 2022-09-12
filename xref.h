#ifndef xref_h
#define xref_h

#include <stdio.h>

#include "pdf.h"
#include "object.h"
#include "typedef.h"

#define ENTRY_WIDTH 20

/**
 * Offset: The byte offset to the object from the start of the file.
 * gen: The generation number of the object.
 * status: Whether the object is deleted or in use.
 *         Either 'n' (in use) or 'f' (deleted).
 */
typedef struct {
  long offset;
  long gen;
  char status;
} x_entry_t;

/**
 * x_offset: byte offset to 'xref' keyword from beginning of the file.
 * t_offset: byte offset to start of table from beginning of the file.
 *           This value should never change.
 * obj_num: The number of the first object in the table.
 * count: The number of entries in the table.
 *        Also corresponds to the number of pointers in the entries array.
 * ce_index: The index of the current entry in the xref table.
 * ce_offset: byte offset from the beginning of the file to the current entry.
 */
struct xref {
  long x_offset;
  long t_offset;
  long obj_num;
  long count;
  long ce_index;
  long ce_offset;
};

/**
 * Parses the xref table.
 * offset: The byte offset to the start of the xref table
 *         which can be found in the trailer.
 * Returns 1 for success. 0 for failure.
 * Adds the xref table to pdf->xref.
 */
int get_xref(pdf_t* pdf);

void free_xref_t(xref_t* x);

void print_xref(xref_t* x);

x_entry_t* read_entry(pdf_t* pdf);

object_t* next_obj(pdf_t* pdf);

object_t* get_object(pdf_t* pdf, int obj_num);

/**
 * Parse and print EVERY entry in the xref table.
 */
void parse_entries(pdf_t*);

#endif // xref_h
