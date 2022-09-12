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
    cexit(fs, 1);
    // return for compiler
    return 0;
  }
}

indirect_t* get_indirect(state_t* state, int c) {
  indirect_t* indirect = allocate(sizeof(indirect_t));
  indirect->obj_num = num;
  indirect->gen_num = gen_num;
  indirect->obj = NULL;

  if (c == 'o') {
    unget_char(fs, c, FAIL);
    skip_string(fs, "obj", get_pos(fs));
    consume_whitespace(fs);
    indirect->obj = next_sym(fs);
    consume_whitespace(fs);

    size_t match = check_for_match_seek_back(fs, "stream");
    if (indirect->obj->type == Dict && match) {
      object_t* len = get_entry_value(indirect->obj, "Length");
      long stream_len = -1;

      if (len->type == Num) {
	stream_len = *(long*)len->val;
	indirect->stream = try_read_stream(fs, stream_len);
      } else if (len->type == Ind) {
	stream_len = get_num_val(
	indirect->stream = try_read_stream(fs, stream_len);
	if (!indirect->stream) {
	  fprintf(stderr, "Warning: Length %li found but stream read failed.\n", stream_len);
	  fprintf(stderr, "  at: %li\n", get_pos(fs));
	}
      } else {
	fprintf(stderr, "Invalid obj type for indirect object: %s\n", get_type_name(indirect->obj));
      }
    }
    consume_whitespace(fs);
    size_t match = check_for_match(fs, "endobj");

    if (!match) {
      fprintf(stderr, "Missing endobj.\n");
      cexit(fs, 1);
    }
  }

  return indirect;
}
