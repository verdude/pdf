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
      break;
    case Stream:
      fprintf(stderr, "Called free object with unhandled object type: %i\n", type);
      break;
    default:
      fprintf(stderr, "free_object: Bad object type: %i\n", type);
      return;
  }

  free(o);
}

void print_term(int v, enum o_type type) {
  switch (type) {
    case Null:
      printf("null ");
      break;
    case Boo:
      printf(v ? "true " : "false ");
      break;
    default:
      printf("called print with unknown term\n");
  }
}

void print_object(object_t* o) {
  enum o_type type = o->type;
  switch (type) {
    case Num:
      printf("%li\n", *(long*)o->val);
      break;
    case Boo:
      print_term(*((int*)o->val), o->type);
      break;
    case Str:
      print_string(o->val, '(', ')');
      break;
    case Hstr:
      print_string(o->val, '<', '>');
      break;
    case Name:
      print_string(o->val, '/', 0);
      break;
    case Dict:
      printf("<<\n");
      print_list(o->val);
      printf(">>\n");
      break;
    case Arr:
      printf("[");
      print_list(o->val);
      printf("]\n");
      break;
    case Ind:
      print_indirect(o->val);
      break;
    case Null:
      print_term(0, o->type);
      break;
    case Stream:
      fprintf(stderr, "Called print with unhandled object type: %i\n", type);
      break;
    default:
      fprintf(stderr, "Bad object type (print_object): %i\n", type);
  }
}

object_t* get_term(state_t* state, enum term type) {
  char* false_str = "false";
  char* true_str = "true";
  char* null_str = "null";
  char* str;
  int val;

  switch (type) {
    case FalseTerm:
      str = false_str;
      val = 0;
      break;
    case TrueTerm:
      str = true_str;
      val = 1;
      break;
    case NullTerm:
      str = null_str;
      break;
    default:
      fprintf(stderr, "Unknown term type: %i\n", type);
      return NULL;
  }

  long offset = get_pos(state->fs);
  int success = check_for_match(state->fs, str);
  if (!success) {
    fprintf(stderr, "Expected '%s' at %li.\n", str, get_pos(state->fs));
    return NULL;
  }

  object_t* o = allocate(sizeof(object_t));
  o->type = type == NullTerm ? Null : Boo;
  o->len = 4;
  o->offset = offset;
  o->val = NULL;

  if (type == TrueTerm || type == FalseTerm) {
    o->val = allocate(sizeof(int));
    *(int*)o->val = val;
  }

  return o;
}

char* get_type_name(object_t* o) {
  switch (o->type) {
    case Boo:
      return "Boolean";
    case Num:
      return "Number";
    case Str:
      return "String";
    case Hstr:
      return "HexString";
    case Name:
      return "NameString";
    case Arr:
      return "Array";
    case Dict:
      return "Dictionary";
    case Null:
      return "Null";
    case Stream:
      return "Stream";
    case Ind:
      return "Indirect";
    default:
      return "invalid type";
  }
}
