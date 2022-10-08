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
  if (o->type != Ind) {
    fprintf(stderr, "Invalid type for get_stream_len: %s\n", get_type_name(o));
    return 0;
  }
  indirect_t* ind = o->val;
  object_t* length_obj = get_entry_value(ind->obj, "Length");

  if (!length_obj) {
    return 0;
  }

  if (length_obj->type == Ind) {
    indirect_t* len_ref = length_obj->val;
    object_t* obj = len_ref->obj;
    if (obj) {
      if (obj->type != Num) {
        fprintf(stderr, "Bad Length Object type for stream length: %s\n", get_type_name(obj));
        scexit(pdf, 1);
      }
      return get_num_val(len_ref->obj);
    } else {
      fprintf(stderr, "um\n");
      scexit(pdf, 1);
    }
  } else if (length_obj->type == Num) {
    return get_num_val(length_obj->val);
  } else {
    scexit(pdf, 1);
    // return for compiler
    return 0;
  }
  return 0;
}

indirect_t* get_indirect(pdf_t* pdf, int nc, long on, long gn) {
  indirect_t* indirect = allocate(sizeof(indirect_t));
  indirect->obj_num = on;
  indirect->gen_num = gn;
  indirect->obj = NULL;

  if (nc == 'o') {
    printf("Getting indirect object at %li\n", get_pos(pdf->fs));
    unget_char(pdf->fs, nc, FAIL);
    skip_string(pdf->fs, "obj", get_pos(pdf->fs));
    consume_whitespace(pdf->fs);
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
        printf("Stream with indirect length encountered...\n");
        stream_len = get_stream_len(pdf, len);
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
      scexit(pdf, 1);
    } else {
      printf("Got endobj at pos: %li\n", get_pos(pdf->fs));
    }
  }

  return indirect;
}
