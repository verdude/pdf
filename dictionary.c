#include <stdio.h>
#include <stdlib.h>

#include "next.h"
#include "object.h"

void print_d_entry(d_entry_t* d) {
  object_t* key = d->key;
  object_t* val = d->val;

  printf("/%s ", ((string_t*) key->val)->str);
  print_object(val);
}

d_entry_t* get_entry(FILE* fs) {
  object_t* first_key = get_name(fs, FAIL, DictionaryEntry);

  d_entry_t* list_t = allocate(sizeof(d_entry_t));
  list_t->key = first_key;
  list_t->val = next_sym(fs);

  return list_t;
}

