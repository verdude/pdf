#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#include "next.h"

object_t* get_number(FILE* fs) {
  string_t* num = allocate(sizeof(string_t));
  int size = 10;
  num->str = allocate(size);
  num->memsize = size;
  num->len = 0;

  object_t* num_obj = allocate(sizeof(object_t));
  num_obj->type = Num;
  num_obj->offset = get_pos(fs);
  num_obj->len = 0;

  int c;
  while ((c = get_char(fs, FAIL))) {
    if (!isdigit(c)) {
      unget_char(fs, c, FAIL);
      break;
    }

    int success = add_byte(c, num);
    if (success) {
      num_obj->len++;
    }
  }

  long value = strtol(num->str, NULL, 10);
  if ((value == LONG_MIN || value == LONG_MAX)) {
    fprintf(stderr, "Failed converting number to long: %s\n", num->str);
    perror("strtol");
    cexit(fs, 1);
  }

  num_obj->val = allocate(sizeof(long));
  *((long*)num_obj->val) = value;

  free(num->str);
  free(num);
  return num_obj;
}

