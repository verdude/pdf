#include <stdio.h>

typedef struct {
} xref_t;

/**
 * Parses the xref table.
 * offset: The byte offset to the start of the xref table
 *         which can be found in the trailer.
 */
xref_t* get_xref(FILE* fs, long offset);

