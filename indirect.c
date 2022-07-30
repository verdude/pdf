#include <stdio.h>

#include "object.h"

void print_indirect(indirect_t* i) {
  printf("%li %li ", i->obj_num, i->gen_num);
  if (i->obj == NULL) {
    printf("R\n");
  } else {
    print_object(i->obj);
  }
}

