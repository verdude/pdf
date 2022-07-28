#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

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

void free_indirect_object(indirect_t* i) {
  if (i->obj) {
    free_object_t(i->obj);
  }
  free(i);
}

void free_list_t(list_t* l) {
  for (int i = 0; i < l->len; i++) {
    switch (l->el_type) {
      case DictionaryEntry:
        free_d_entry_t(l->el[i]);
        break;
      case Object:
        free_object_t(l->el[i]);
        break;
      default:
        fprintf(stderr, "Invalid list type: %i\n", l->el_type);
    }
  }
  free(l->el);
  free(l);
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
    case Ind:
      free_indirect_object(o->val);
      break;
    case Null:
    case Stream:
      fprintf(stderr, "Called free object with unhandled object type: %i\n", type);
      break;
    default:
      fprintf(stderr, "free_object: Bad object type: %i\n", type);
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
      printf("<<\n");
      print_list(o->val);
      printf(">>\n");
      break;
    case Arr:
      print_list(o->val);
      break;
    case Ind:
      print_indirect(o->val);
      break;
    case Null:
    case Stream:
      fprintf(stderr, "Called print with unhandled object type: %i\n", type);
      break;
    default:
      fprintf(stderr, "Bad object type (print_object): %i\n", type);
      return;
  }
}
