#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

object_t* get_string_type_obj(FILE* fs, unsigned char first_char, int fail_on_error) {
  consume_whitespace(fs);
  int c = get_char(fs, FAIL);
  if (c != first_char && fail_on_error) {
    printf("Invalid first char for object: [%c]. "
        "Must begin with %c.\n", c, first_char);
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

  return obj;
}

