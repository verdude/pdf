#include <stdio.h>
#include <stdlib.h>

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
      case Dict:
          print_dictionary((dict_t*) val->val);
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

// TODO: Convert this and add_byte to generic data realloc/appender
// for code reuse. Bad idea? Or Nah?
static int add_entry(dict_t* dict, d_entry_t* entry) {
  size_t ptr_width = sizeof(d_entry_t*);
  int new_size = dict->memsize;

  while (new_size / ptr_width < dict->len) {
    new_size += dict->memsize;
  }

  if (new_size < 0) {
    fprintf(stderr, "add_entry failed, overflow.\n");
    return 0;
  }

  if (new_size > dict->memsize) {
    d_entry_t** reallocd_entries = realloc(dict->entries, new_size);
    if (reallocd_entries == NULL) {
      perror("realloc in add_entry");
      return 0;
    }
    dict->entries = reallocd_entries;
  }

  dict->memsize = new_size;
  dict->entries[dict->len] = entry;
  dict->len++;
  return 1;
}

object_t* get_dictionary(FILE* fs, int fail_on_error) {
  size_t pos = get_pos(fs);

  d_entry_t* entry = get_entry(fs, fail_on_error);

  size_t size = sizeof(d_entry_t*) * 10;
  dict_t* dict = allocate(sizeof(dict_t));
  dict->entries = allocate(size);
  dict->memsize = size;

  int success = add_entry(dict, entry);
  if (!success) {
    // TODO: free dict..
    cexit(fs, 1);
  }

  object_t* obj = allocate(sizeof(object_t));
  obj->type = Dict;
  // points to the first char after dictionary sym '<<'
  obj->offset = pos;
  obj->len = 0;
  obj->val = (void*) dict;

  return obj;
}

