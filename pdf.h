#ifndef pdf_h
#define pdf_h

#include <stdio.h>

#include "typedef.h"

/**
 * The PDF pdf.
 */
typedef struct {
  FILE* fs;
  xref_t* xref;
  trailer_t* trailer;
} pdf_t;

void free_pdf_t(pdf_t* pdf);

#endif // pdf_h
