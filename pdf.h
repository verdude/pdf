#ifndef pdf_h
#define pdf_h

#include <stdio.h>

#include "typedef.h"

/**
 * The PDF state.
 */
typedef struct {
  FILE* fs;
  xref_t* xref;
  trailer_t* trailer;
} state_t;

void free_state_t(state_t* state);

#endif // pdf_h
