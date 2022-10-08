#include <stdio.h>

#include "object.h"
#include "next.h"

static int is_hex_char(char c) {
  if (('A' <= c && c <= 'F') || ('a' <= c && c <= 'f') || ('0' <= c && c <= '9')) {
    return 1;
  }
  return 0;
}

static int add_hex_string_char(pdf_t* pdf, int c, string_t* hstring) {
  int c2 = get_char(pdf, FAIL);
  if (is_hex_char(c) && is_hex_char(c2)) {
    int success = add_byte(c, hstring) & add_byte(c2, hstring);
    if (!success) {
      scexit(pdf, 1);
    }
    return 2;
  }

  unget_char(pdf, c2, FAIL);
  return 0;
}

object_t* get_hex_string(pdf_t* pdf) {
  object_t* obj = get_string_type_obj(pdf, HexString);
  string_t* hstring = obj->val;

  int c;
  while ((c = get_char(pdf, FAIL)) != EOF) {
    int char_len = add_hex_string_char(pdf, c, hstring);

    if (!char_len) {
      break;
    } else {
      obj->len += char_len;
    }
  }

  if (c != '>') {
    printf("Bad char in hex string: %c\n", c);
    scexit(pdf, 1);
  }

  return obj;
}

