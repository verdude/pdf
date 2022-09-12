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
static int add_string_char(pdf_t* pdf, int c, string_t* string) {
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
      int escaped = get_char(pdf->fs, FAIL);
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

object_t* get_string_type_obj(pdf_t* pdf, enum encoding enc) {
  consume_whitespace(pdf->fs);
  int c = get_char(pdf->fs, FAIL);
  if (!first_char(enc, c)) {
    fprintf(stderr, "Invalid first char for string: [%c] aka. [0x%04x]. Should be: %c\n",
        c, c, get_first_char(enc));
    cexit(pdf->fs, 1);
  }

  object_t* obj = allocate(sizeof(object_t));
  obj->offset = get_pos(pdf->fs);
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
      scexit(pdf, 1);
  }

  string_t* val = obj->val;
  val->memsize = 1;
  val->str = allocate(val->memsize);
  val->enc = enc;

  return obj;
}

object_t* get_string(pdf_t* pdf) {
  object_t* string = get_string_type_obj(pdf, LiteralString);

  int c;
  while ((c = get_char(pdf->fs, FAIL)) != EOF) {
    int char_len = get_char(pdf->fs, FAIL);

    if (char_len == -1) {
      break;
    }

    if (!char_len) {
      break;
    }
    string->len += char_len;
    int success = add_string_char(pdf, c, string->val);
    if (success == -1) {
      break;
    } else if (!success) {
      free_object_t(string);
      scexit(pdf, 1);
    }
  }

  return string;
}

void print_string(string_t* s, char open, char close) {
  printf("%c%s%c\n", open, s->str, close);
}

