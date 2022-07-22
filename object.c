#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

object_t* get_dictionary(FILE* fs, int fail_on_error) {
  size_t pos = get_pos(fs);
  object_t* first_key = get_name(fs, fail_on_error);

  d_entry_t* entry = allocate(sizeof(d_entry_t));
  entry->key = first_key;
  entry->val = NULL;

  dict_t* dict = allocate(sizeof(dict_t));
  dict->entries = allocate(sizeof(d_entry_t*));
  dict->entries[0] = entry;
  dict->len = 1;

  object_t* obj = allocate(sizeof(object_t));
  obj->type = Dict;
  // points to the first char after dictionary sym '<<'
  obj->offset = pos;
  obj->len = 0;
  obj->val = (void*) dict;

  return obj;
}

void print_dictionary(dict_t* d) {
  for (int i = 0; i < d->len; ++i) {
    printf("KEY: (%s) VAL: %p\n", (unsigned char*) ((string_t*) d->entries[i]->key->val)->str, d->entries[i]->val);
  }
}

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

