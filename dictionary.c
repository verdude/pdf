#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

object_t* get_entry_value(object_t* o, char* key) {
  if (o->type != Dict) {
    fprintf(stderr, "Invalid object for dictionary#has_key: %s\n", get_type_name(o));
    return NULL;
  }
  list_t* dict = o->val;
  d_entry_t** entries = dict->el;
  int len = strlen(key);

  for (int i = 0; i < dict->len; ++i) {
    if (string_equals(entries[i]->key, key, len) == 0) {
      return entries[i]->val;
    }
  }
  return NULL;
}

void print_d_entry(d_entry_t* d) {
  object_t* key = d->key;
  object_t* val = d->val;

  printf("/%s ", ((string_t*) key->val)->str);
  print_object(val);
}

d_entry_t* get_entry(FILE* fs) {
  object_t* first_key = get_name(fs, FAIL);

  d_entry_t* list_t = allocate(sizeof(d_entry_t));
  list_t->key = first_key;
  list_t->val = next_sym(fs);

  return list_t;
}
