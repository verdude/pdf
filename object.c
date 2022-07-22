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
    printf("KEY: (%s) VAL: %p\n", (unsigned char*) ((name_t*) d->entries[i]->key->val)->str, d->entries[i]->val);
  }
}

