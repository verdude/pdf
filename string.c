#include <stdio.h>
#include <string.h>

#include "next.h"
#include "object.h"

char* get_string_type(string_t* s) {
  switch (s->enc) {
    case LiteralString:
      return "LiteralString";
    case HexString:
      return "HexString";
    case NameString:
      return "NameString";
    default:
      return "invalid string";
  }
}

int string_equals(object_t* o, char* s, int n) {
  switch (o->type) {
    case Str:
    case Hstr:
    case Name:
      break;
    default:
      fprintf(stderr, "Bad object for string_equals: %s\n", get_type_name(o));
      return -1;
  }
  string_t* string = o->val;

  return strncmp(string->str, s, n);
}

/**
 * Returns 1 on success, 0 on failure, -1 on string end
 */
static int add_string_char(state_t* state, int c, string_t* string) {
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
      // Ok in literal
      // fail in name string
      return string->enc == HexString ? -1 : string->enc == LiteralString ? 1 : 0;
    case 0x5c:
      // \ backslash
      int escaped = get_char(fs, FAIL);
      return add_byte(escaped, string);
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

int get_first_char(enum encoding enc) {
  switch (enc) {
    case HexString:
      return '<';
    case LiteralString:
      return '(';
    case NameString:
      return '/';
    default:
      return 0;
  }
}

object_t* get_string_type_obj(state_t* state, enum encoding enc) {
  consume_whitespace(fs);
  int c = get_char(fs, FAIL);
  if (!first_char(enc, c)) {
    fprintf(stderr, "Invalid first char for string: [%c]. Should be: %c\n",
        c, get_first_char(enc));
    cexit(fs, 1);
  }

  object_t* obj = allocate(sizeof(object_t));
  obj->offset = get_pos(fs);
  obj->len = 0;
  obj->val = allocate(sizeof(string_t));
  switch (enc) {
    case NameString:
      obj->type = Name;
      break;
    case HexString:
      obj->type = Hstr;
      break;
    case LiteralString:
      obj->type = Str;
      break;
    default:
      fprintf(stderr, "Invalid string encoding type: %i\n", enc);
      cexit(fs, 1);
  }

  string_t* val = obj->val;
  val->memsize = 1;
  val->str = allocate(val->memsize);
  val->enc = enc;

  return obj;
}

object_t* get_string(state_t* state) {
  object_t* string = get_string_type_obj(fs, LiteralString);

  string_t* s = (string_t*) string->val;
  int c;

  while ((c = get_char(fs, FAIL)) != EOF) {
    int char_len = add_string_char(fs, c, s);

    if (char_len == -1) {
      break;
    }

    if (!char_len) {
      break;
    }
    string->len += char_len;
  }

  return string;
}

void print_string(string_t* s, char open, char close) {
  printf("%c%s%c\n", open, s->str, close);
}

