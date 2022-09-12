#include <stdio.h>

#include "object.h"
#include "next.h"

void print_indirect(indirect_t* i) {
  printf("%li %li ", i->obj_num, i->gen_num);
  if (i->obj == NULL) {
    printf("R\n");
  } else {
    print_object(i->obj);
  }
}

long get_stream_len(state_t* state, object_t* o) {
  indirect_t* ind = o->val;
  object_t* length_obj = get_entry_value(ind->obj, "Length");

  if (!length_obj) {
    return 0;
  }

  if (length_obj->type == Ind) {
    // get Ind obj
    return 0;
  } else if (length_obj->type == Num) {
    return get_num_val(length_obj->val);
  } else {
    cexit(state->fs, 1);
    // return for compiler
    return 0;
  }
}

indirect_t* get_indirect(state_t* state, int c) {
  indirect_t* indirect = allocate(sizeof(indirect_t));
  indirect->obj_num = 0; //TODO
  indirect->gen_num = 0; // TODO
  indirect->obj = NULL;

  if (c == 'o') {
    unget_char(state->fs, c, FAIL);
    skip_string(state, "obj", get_pos(state->fs));
    consume_whitespace(state);
    indirect->obj = next_sym(state);
    consume_whitespace(state);

    size_t match = check_for_match_seek_back(state, "stream");
    if (indirect->obj->type == Dict && match) {
      object_t* len = get_entry_value(indirect->obj, "Length");
      long stream_len = -1;

      if (len->type == Num) {
  stream_len = *(long*)len->val;
  indirect->stream = try_read_stream(state, stream_len);
      } else if (len->type == Ind) {
        //stream_len = get_num_val(); // TODO !!!!
        indirect->stream = try_read_stream(state, stream_len);
        if (!indirect->stream) {
          fprintf(stderr, "Warning: Length %li found but stream read failed.\n", stream_len);
          fprintf(stderr, "  at: %li\n", get_pos(state->fs));
        }
      } else {
  fprintf(stderr, "Invalid obj type for indirect object: %s\n", get_type_name(indirect->obj));
      }
    }
    consume_whitespace(state);
    match = check_for_match(state->fs, "endobj");

    if (!match) {
      fprintf(stderr, "Missing endobj.\n");
      cexit(state->fs, 1);
    }
  }

  return indirect;
}
