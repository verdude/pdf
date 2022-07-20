#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

static int valid_name_char(FILE* fs, int c) {
  if (c > 0x21 && c < 0x7e) {
    return c;
  }

  if (c == '#') {
    int c1 = get_char(fs, FAIL);
    int c2 = get_char(fs, FAIL);
    char hex_str[] = { (char) c1, (char) c2 };

    long abnormal = strtol(hex_str, NULL, 16);
    if (abnormal == LONG_MAX) {
      perror("valid_name_char");
      cexit(fs, 1);
    }

    return (int) abnormal;
  } else {
    fprintf(stderr, "Invalid char in name: %#4x", c);
    return 0;
  }
}

object_t* get_name(FILE* fs, int fail_on_error) {
  int c = get_char(fs, FAIL);
  if (c != '/' && fail_on_error) {
    printf("Invalid name char: %c\n", c);
    cexit(fs, 1);
  }

  object_t* name = allocate(sizeof(object_t));
  name->offset = get_pos(fs);
  name->len = 0;
  name->type = o_type.name;

  while ((c = get_char(fs, FAIL)) && (c = valid_name_char(fs, c))) {
    name->len += c;
  }

  return name;
}

char* name_str(FILE* fs, object_t* name) {
  long cpos = get_pos(fs);
  char* ns = allocate(name->len);
  seek(fs, name->offset, FAIL);

  return ns;
}

