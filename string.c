#include <stdio.h>

#include "next.h"
#include "object.h"

/**
 * Returns 1 on success, 0 on failure, -1 on string end
 */
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
      // End of string
      return string->enc == LiteralString ? -1 : 0;
    case 0x3e:
      // >
      // End of hex string
      return string->enc == HexString ? -1 : 0;
    case 0x5c:
      // \\ backslash
      printf("backslash in string\n");
      // TODO: get control char or paren or octal or escaped backslash
      return 0;
  }

  return add_byte(c, string);
}

int first_char(enum encoding enc, int c) {
  switch (enc) {
    case HexString:
      return c == '<';
    case LiteralString:
      return c == '(';
    case NameString:
      return c == '/';
    default:
      return 0;
  }
}

object_t* get_string_type_obj(FILE* fs, enum encoding enc) {
  consume_whitespace(fs);
  int c = get_char(fs, FAIL);
  if (!first_char(enc, c)) {
    fprintf(stderr, "Invalid first char for string: [%c]. Should be: %c\n",
        c, first_char(enc, c));
    cexit(fs, 1);
  }

  object_t* obj = allocate(sizeof(object_t));
  obj->offset = get_pos(fs);
  obj->len = 0;
  obj->type = Name;
  obj->val = allocate(sizeof(string_t));

  string_t* val = obj->val;
  val->memsize = 1;
  val->str = allocate(val->memsize);
  val->enc = enc;

  return obj;
}

object_t* get_string(FILE* fs, enum encoding enc) {
  object_t* string = get_string_type_obj(fs, enc);

  string_t* s = (string_t*) string->val;
  int c;

  while ((c = get_char(fs, FAIL))) {
    int char_len = add_string_char(fs, c, s);

    if (char_len == -1) {
      break;
    }

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

void print_string(string_t* s) {
  printf("%s\n", s->str);
}

