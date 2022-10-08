#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "next.h"
#include "object.h"

static unsigned char get_hex_char(pdf_t* pdf) {
  int c1 = get_char(pdf, FAIL);
  int c2 = get_char(pdf, FAIL);
  char hex_str[] = { (unsigned char) c1, (unsigned char) c2 };

  // Validate hex_str by converting to long
  long abnormal = strtol(hex_str, NULL, 16);
  if (abnormal == LONG_MAX) {
    fprintf(stderr,
        "Invalid hex character [0x%c%c] at: %li\n",
        c1, c2, get_pos(pdf));
    perror("get_name_char_len");
    return 0;
  }

  return (unsigned char) abnormal;
}

/**
 * Adds a char to ptr+len, reallocating ptr if len >= memsize.
 * Updates memsize and len.
 * returns 0 in case of failure, 1 in case of success.
 */
int add_byte(unsigned char c, string_t* st) {
  int new_size = st->memsize;
  st->str = st->str;
  while (new_size < st->len + 2) {
    new_size += st->memsize;
  }

  if (new_size < 0) {
    fprintf(stderr, "Overflow computing new size: ptr:"
        " %p memsize: %i strlen: %i\n", st->str, st->memsize, st->len);
    return 0;
  }

  if (new_size > st->memsize) {
    st->str = realloc(st->str, new_size);
    if (st->str == NULL) {
      perror("realloc in add_byte");
      return 0;
    }
  }

  st->memsize = new_size;
  st->str[st->len++] = c;
  st->str[st->len] = 0;
  return 1;
}

int is_key_char(int c) {
  switch (c) {
    case 0x2f:
    case 0x5b:
    case 0x3c:
    case 0x3e:
    case 0x5d:
    case 0x28:
    case 0x29:
      return 1;
    default:
      return 0;
  }
}

/**
 * 1 for a regular char.
 * 3 for a hex char, encoded as: #0A in the pdf.
 * 0 for invalid chars. May be the start of another symbol.
 *
 * returns positive int if a char was read successfully.
 * Int signifies the number of bytes that it takes to represent
 * the character in the pdf.
 * returns 0 when done reading.
 */
static int add_name_char(pdf_t* pdf, int c, string_t* name) {
  // TODO: add warning for other object symbols
  // to make debugging invalid dict entries easier.
  // ex:
  //   /ID[<...><...>]
  //   No space, next char is [ which is valid in a name
  //   but not intended to be part of the name in this case.
  if (c > 0x21 && c < 0x7e) {
    if (is_key_char(c)) {
      // Invalid format but assume the pdf
      // contains something like: /Key/Val
      if (c == '(') {
        printf("kicked in the mouth\n");
      }
      return 0;
    }
    int success = add_byte(c, name);
    if (!success) {
      scexit(pdf, 1);
    }
    return 1;
  }

  if (c == '#') {
    unsigned char h_char = get_hex_char(pdf);
    int success = add_byte(h_char, name);
    if (!success) {
      scexit(pdf, 1);
    }
    return 3;
  } else {
    return 0;
  }
}

object_t* get_name(pdf_t* pdf, int fail_on_error) {
  object_t* name_obj = get_string_type_obj(pdf, NameString);
  string_t* name_val = name_obj->val;
  int c;

  while ((c = get_char(pdf, FAIL)) != EOF) {
    int char_len = add_name_char(pdf, c, name_val);

    if (!char_len) {
      // finished reading name
      unget_char(pdf, c, FAIL);
      break;
    } else {
      name_obj->len += char_len;
    }
  }

  return name_obj;
}

