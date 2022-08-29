#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"

int num_char(int c) {
  if (isdigit(c) || c == '.') {
    return 1;
  }

  return 0;
}

long get_num(FILE* fs, int base) {
  const int len = 64;
  char s[64] = {0};
  char decimal[64] = {0};
  consume_chars_stack(fs, &num_char, s, len);
  char* end;
  size_t slen = strnlen(s, len);

  if (!slen) {
    fprintf(stderr, "bad number.\n");
    cexit(fs, 1);
  }

  long n = estrtol(s, &end, base);
  if (*end != 0) {
    size_t extra = strnlen(end, len);
    seek(fs, -extra, SEEK_CUR);
    if (*end == '.') {
      printf("%c\n", get_char(fs, FAIL));
      // read decimal
      consume_chars_stack(fs, &isdigit, decimal, len);
      printf("Decimal string: [%s]\n", decimal);
    }
  } else if (end == s) {
    fprintf(stderr, "whole string it not a num! [%s]\n", s);
    cexit(fs, 1);
  }

  return n;
}

object_t* create_num_obj(FILE* fs, long start, long num) {
  object_t* o = allocate(sizeof(object_t));
  o->type = Num;
  o->offset = start;
  o->len = get_pos(fs) - start;
  o->val = allocate(sizeof(long));
  *((long*)o->val) = num;

  return o;
}

object_t* parse_num(FILE* fs) {
  long cpos = get_pos(fs);
  long pos = cpos;
  long num = get_num(fs, 0);
  int c = get_char(fs, FAIL);
  cpos = get_pos(fs);

  if (c != ' ') {
    unget_char(fs, c, FAIL);
    return create_num_obj(fs, pos, num);
  }

  long gen_num = get_num(fs, 0);
  c = get_char(fs, FAIL);
  if (c != ' ') {
    seek(fs, cpos, SEEK_SET);
    return create_num_obj(fs, pos, num);
  }

  c = get_char(fs, FAIL);
  if (c != 'R' && c != 'o') {
    seek(fs, cpos, SEEK_SET);
    return create_num_obj(fs, pos, num);
  }

  // Is indirect object
  indirect_t* indirect = allocate(sizeof(indirect_t));
  indirect->obj_num = num;
  indirect->gen_num = gen_num;
  indirect->obj = NULL;

  if (c == 'o') {
    unget_char(fs, c, FAIL);
    skip_string(fs, "obj", get_pos(fs));
    consume_whitespace(fs);
    indirect->obj = next_sym(fs);
  }

  object_t* o = allocate(sizeof(object_t));
  o->type = Ind;
  o->offset = pos;
  o->len = get_pos(fs) - pos;
  o->val = indirect;

  return o;
}

