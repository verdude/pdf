#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "log.h"

int num_char(int c) {
  if (isdigit(c) || c == '.') {
    return 1;
  }

  return 0;
}

long get_num(pdf_t* pdf, int base, int fail_on_error) {
  const int len = 64;
  char s[64] = {0};
  char decimal[64] = {0};
  consume_chars_stack(pdf, &num_char, s, len);
  char* end;
  size_t slen = strnlen(s, len);

  if (!slen && fail_on_error) {
    log_e("bad number. [0x%09x]", s);
    scexit(pdf, 1);
  } else if (!slen) {
    return -1;
  }

  long n = estrtol(s, &end, base);
  if (*end != 0) {
    size_t extra = strnlen(end, len);
    seek(pdf, -extra, SEEK_CUR);
    if (*end == '.') {
      get_char(pdf, FAIL);
      // read decimal
      consume_chars_stack(pdf, &isdigit, decimal, len);
    }
  } else if (end == s) {
    log_e("whole string is not a num! [%s]", s);
    scexit(pdf, 1);
  }

  return n;
}

object_t* create_num_obj(pdf_t* pdf, long start, long num) {
  object_t* o = allocate(sizeof(object_t));
  o->type = Num;
  o->offset = start;
  o->len = get_pos(pdf) - start;
  o->val = allocate(sizeof(long));
  *((long*)o->val) = num;

  return o;
}

long get_num_val(object_t* o) {
 return *((long*)o->val);
}

object_t* parse_num(pdf_t* pdf) {
  long cpos = get_pos(pdf);
  long pos = cpos;
  long num = get_num(pdf, 0, FAIL);
  int c = get_char(pdf, FAIL);
  cpos = get_pos(pdf);

  if (c != ' ') {
    unget_char(pdf, c, FAIL);
    return create_num_obj(pdf, pos, num);
  }

  long gen_num = get_num(pdf, 0, IGNORE);
  c = get_char(pdf, FAIL);
  if (c != ' ' || gen_num < 0) {
    seek(pdf, cpos, SEEK_SET);
    return create_num_obj(pdf, pos, num);
  }

  c = get_char(pdf, FAIL);
  if (c != 'R' && c != 'o') {
    seek(pdf, cpos, SEEK_SET);
    return create_num_obj(pdf, pos, num);
  }

  object_t* o = allocate(sizeof(object_t));
  o->type = Ind;
  o->offset = pos;
  o->len = get_pos(pdf) - pos;
  o->val = get_indirect(pdf, c, num, gen_num);

  return o;
}
