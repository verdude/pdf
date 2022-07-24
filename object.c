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

void free_num_boo(void* n) {
  free(n);
}

void free_string_t(string_t* s) {
  free(s->str);
  free(s);
}

void free_d_entry_t(d_entry_t* e) {
  free_object_t(e->key);
  free_object_t(e->val);
  free(e);
}

void free_list_t(list_t* d) {
  for (int i = 0; i < d->len; i++) {
    switch (d->el_type) {
      case DictionaryEntry:
        free_d_entry_t(d->el[i]);
        break;
      case Object:
        free_object_t(d->el[i]);
        break;
      default:
        fprintf(stderr, "WOW. Very strange.");
    }
  }
  free(d->el);
  free(d);
}

void free_object_t(object_t* o) {
  enum o_type type = o->type;
  switch (type) {
    case Num:
    case Boo:
      free_num_boo(o->val);
      break;
    case Str:
    case Hstr:
    case Name:
      free_string_t(o->val);
      break;
    case Dict:
    case Arr:
      free_list_t(o->val);
      break;
    case Null:
    case Stream:
    case Ind:
      fprintf(stderr, "Called free with unhandled object type: %i\n", type);
      break;
    default:
      fprintf(stderr, "Bad object type: %i\n", type);
      return;
  }

  free(o);
}

void print_object(object_t* o) {
  enum o_type type = o->type;
  switch (type) {
    case Num:
    case Boo:
      printf("%li\n", *(long*)o->val);
      break;
    case Str:
    case Hstr:
    case Name:
      print_string(o->val);
      break;
    case Dict:
    case Arr:
      print_list(o->val);
      break;
    case Null:
    case Stream:
    case Ind:
      fprintf(stderr, "Called print with unhandled object type: %i\n", type);
      break;
    default:
      fprintf(stderr, "Bad object type (print_object): %i\n", type);
      return;
  }
}
