#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

static char valid_name_char(FILE* fs, int c) {
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

    return (char) abnormal;
  } else {
    fprintf(stderr, "Invalid char in name: %#4x", c);
    return 0;
  }
}

name_t get_name(FILE* fs, int fail_on_error) {
  int c = get_char(fs, FAIL);
  if (c != '/' && fail_on_error) {
    printf("Invalid name char: %c\n", c);
    cexit(fs, 1);
  }

  int len = 1024;
  name_t name = calloc(len, 1);
  size_t i = 0;
  while (i < len - 1 && (c = get_char(fs, FAIL)) && (c = valid_name_char(fs, c))) {
    name[i++] = c;
  }

  return name;
}

