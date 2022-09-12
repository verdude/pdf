#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

void print_list(list_t* l) {
  for (int i = 0; i < l->len; ++i) {
    switch (l->el_type) {
      case Object:
        print_object(l->el[i]);
        break;
      case DictionaryEntry:
        print_d_entry(l->el[i]);
        break;
      default:
        fprintf(stderr, "Bad List type: %i\n", l->el_type);
        return;
    }
  }
}

int add_obj_to_list(list_t* list, void* ptr) {
  size_t ptr_width = sizeof(void*);
  int new_size = list->memsize;

  while (new_size / ptr_width <= list->len) {
    new_size += list->memsize;
  }

  if (new_size < 0) {
    fprintf(stderr, "add_obj_to_list failed, overflow.\n");
    return 0;
  }

  if (new_size > list->memsize) {
    void** reallocd_entries = realloc(list->el, new_size);
    if (reallocd_entries == NULL) {
      perror("realloc in add_obj_to_list");
      return 0;
    }
    list->el = reallocd_entries;
  }

  list->memsize = new_size;
  list->el[list->len] = ptr;
  list->len++;
  return 1;
}

list_t* create_list(enum el_t el_type) {
  size_t size = sizeof(void*) * 10;

  list_t* list = allocate(sizeof(list_t));
  list->el_type = el_type;
  list->el = allocate(size);
  list->memsize = size;
  list->len = 0;

  return list;
}

object_t* get_list(state_t* state, enum el_t el_type) {
  size_t pos = get_pos(state->fs);
  size_t end = pos;

  list_t* list = create_list(el_type);

  char* terminator;
  read_element re;

  if (list->el_type == DictionaryEntry) {
    terminator = ">>";
    // Needs to consume these chars to get into position
    check_for_match(state->fs, "<<");
    re = (read_element) &get_entry;
  } else {
    terminator = "]";
    check_for_match(state->fs, "[");
    re = (read_element) &next_sym;
  }

  consume_whitespace(state->fs);

  while (!(end = check_for_match_seek_back(state->fs, terminator))) {
    void* element = (*re)(state);

    if (element == NULL) {
      fprintf(stderr, "Failed to get object\n");
      cexit(state->fs, 1);
    }

    int success = add_obj_to_list(list, element);
    if (!success) {
      fprintf(stderr, "not success! adding object to list\n");
    }
    consume_whitespace(state->fs);
  }

  // skip terminator
  seek(state->fs, strlen(terminator), SEEK_CUR);

  object_t* obj = allocate(sizeof(object_t));
  obj->type = el_type == DictionaryEntry ? Dict : Arr;
  // points to the first char in init sym ("<<" or "[")
  obj->offset = pos;
  obj->len = end - pos;
  obj->val = list;

  return obj;
}

