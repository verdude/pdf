#include <stdio.h>

#include "object.h"

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
 * Offset: byte offset to start of table from beginning of the file.
 * obj_num: The number of the first object in the table.
 * count: The number of entries in the table.
 *        Also corresponds to the number of pointers in the entries array.
 * index: An offset to the current entry.
 */
typedef struct {
  long offset;
  long obj_num;
  long count;
  long index;
} xref_t;

/**
 * Parses the xref table.
 * offset: The byte offset to the start of the xref table
 *         which can be found in the trailer.
 */
xref_t* get_xref(FILE* fs, long offset);

void free_xref_t(xref_t* x);

void print_xref(xref_t* x);

