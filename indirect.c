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

long get_stream_len(pdf_t* pdf, object_t* o) {
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
    cexit(pdf->fs, 1);
    // return for compiler
    return 0;
  }
}

indirect_t* get_indirect(pdf_t* pdf, int nc, long on, long gn) {
  printf("entering experimental territory...\n");
  indirect_t* indirect = allocate(sizeof(indirect_t));
  indirect->obj_num = on;
  indirect->gen_num = gn;
  indirect->obj = NULL;

  if (nc == 'o') {
    unget_char(pdf->fs, nc, FAIL);
    skip_string(pdf->fs, "obj", get_pos(pdf->fs));
    consume_whitespace(pdf->fs);
    printf("Getting next_sym...\n");
    indirect->obj = next_sym(pdf);
    consume_whitespace(pdf->fs);

    size_t match = check_for_match_seek_back(pdf->fs, "stream");
    if (indirect->obj->type == Dict && match) {
      object_t* len = get_entry_value(indirect->obj, "Length");
      long stream_len = -1;

      if (len->type == Num) {
        stream_len = *(long*)len->val;
        indirect->stream = try_read_stream(pdf, stream_len);
      } else if (len->type == Ind) {
        printf("Stream encountered... will fail without a doubt.\n");
        //stream_len = get_num_val(); // TODO !!!!
        indirect->stream = try_read_stream(pdf, stream_len);
        if (!indirect->stream) {
          fprintf(stderr, "Warning: Length %li found but stream read failed.\n", stream_len);
          fprintf(stderr, "  at: %li\n", get_pos(pdf->fs));
        }
      } else {
        fprintf(stderr, "Invalid obj type for indirect object: %s\n", get_type_name(indirect->obj));
      }
    }
    consume_whitespace(pdf->fs);
    match = check_for_match(pdf->fs, "endobj");

    if (!match) {
      fprintf(stderr, "Missing endobj.\n");
      cexit(pdf->fs, 1);
    } else {
      printf("Got endobj at pos: %li\n", get_pos(pdf->fs));
    }
  }

  return indirect;
}
