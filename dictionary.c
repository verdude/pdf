#include <stdio.h>
#include <stdlib.h>

#include "next.h"
#include "object.h"

void print_dictionary_entry(d_entry_t* d) {
  object_t* key = d->key;
  object_t* val = d->val;
  // type of the value
  enum o_type type = val->type;

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
    case Dict:
        print_list((list_t*) val->val);
        break;
    default:
        printf("unhandled type: %i\n", type);
  }
}

d_entry_t* get_entry(FILE* fs) {
  object_t* first_key = get_name(fs, FAIL);

  d_entry_t* list_t = allocate(sizeof(d_entry_t));
  list_t->key = first_key;
  list_t->val = next_sym(fs);

  return list_t;
}

