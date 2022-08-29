#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

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

object_t* get_val(list_t* dict, char* key) {
  if (dict->el_type != DictionaryEntry) {
    printf("obj is not a dict.\n");
    cexit(NULL, 1);
  }

  for (int i = 0; i < dict->len; ++i) {
    d_entry_t* entry = ((d_entry_t*) dict->el[i]);
    string_t* ckey = entry->key->val;
    if (strncmp(key, ckey->str, ckey->len+1) == 0) {
      return entry->val;
    }
  }
  return NULL;
}

