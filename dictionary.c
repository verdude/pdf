#include <stdio.h>

#include "next.h"
#include "object.h"

void print_dictionary(dict_t* d) {
  for (int i = 0; i < d->len; ++i) {
    d_entry_t* entry = d->entries[i];
    object_t* key = entry->key;
    object_t* val = entry->val;
    // type of the value
    enum o_type type = entry->val->type;

    printf("KEY: (%s) ", ((string_t*) key->val)->str);

    switch (type) {
      case Str:
      case Hstr:
      case Name:
        printf("VAL: %s\n", ((string_t*) val->val)->str);
        break;
      case Num:
      case Boo:
          printf("VAL: %li\n", *((long*) val->val));
          break;
      case Null:
          printf("VAL: null\n");
          break;
      default:
          printf("unhandled type: %i\n", type);
    }
  }
}

static d_entry_t* get_entry(FILE* fs, int fail_on_error) {
  object_t* first_key = get_name(fs, fail_on_error);

  d_entry_t* entry = allocate(sizeof(d_entry_t));
  entry->key = first_key;
  entry->val = next_sym(fs);

  return entry;
}

object_t* get_dictionary(FILE* fs, int fail_on_error) {
  size_t pos = get_pos(fs);

  d_entry_t* entry = get_entry(fs, fail_on_error);

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

