#include <stdio.h>

#include "object.h"

/**
 * Offset: byte offset to start of table from beginning of the file.
 * obj_num: The number of the first object in the table.
 * entries: Array of pointers to x_entry_t.
 * count: The number of entries in the table.
 *        Also corresponds to the number of pointers in the entries array.
 * memsize: The size of the allocated space for the entries array.
 */
typedef struct {
  long offset;
  long obj_num;
  list_t** entries;
  long count;
  int memsize;
} xref_t;

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
 * Parses the xref table.
 * offset: The byte offset to the start of the xref table
 *         which can be found in the trailer.
 */
xref_t* get_xref(FILE* fs, long offset);

void free_xref_t(xref_t* x);

