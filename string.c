#include <stdio.h>

#include "next.h"
#include "object.h"

static int add_string_char(FILE* fs, int c, string_t* string) {
  switch (c) {
    case 0x8:
      // backspace
      printf("backspace literal in string\n");
      return 0;
    case 0xa:
      // newline
      printf("newline literal in string\n");
      return 0;
    case 0x9:
      // horizontal tab
      printf("htab literal in string\n");
      return 0;
    case 0xd:
      // carriage return
      printf("cr literal in string\n");
      return 0;
    case 0xc:
      // form feed
      printf("ff literal in string\n");
      return 0;
    case 0x28:
      // (
      printf("open paren in string\n");
      return 0;
    case 0x29:
      // )
      printf("close paren in string\n");
      return 0;
    case 0x5c:
      // \\ backslash
      printf("backslash in string\n");
      // TODO: get control char or paren or octal or escaped backslash
      return 0;
  }

  return add_byte(c, string);
}

object_t* get_string(FILE* fs, int fail_on_error) {
  object_t* string = get_string_type_obj(fs, '(', fail_on_error);

  string_t* s = (string_t*) string->val;
  int c;

  while ((c = get_char(fs, FAIL))) {
    int char_len = add_string_char(fs, c, s);

    if (!char_len) {
      // finished reading name
      unget_char(fs, c, FAIL);
      break;
    } else {
      string->len += char_len;
    }
  }

  return string;
}

