#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "next.h"

FILE* file_exists(char* path) {
  FILE* fs = fopen(path, "r");
  if (fs) {
    return fs;
  }

  perror(path);
  return 0;
}

int version1_7(FILE* fs) {
  const int header_len = 8;
  const char* header = "%PDF-1.7";
  char bytes[header_len+1];
  char c;

  for (size_t i = 0; i < header_len; ++i) {
    c = get_char(fs, 1);
    bytes[i] = (char) c;

    if (c != header[i]) {
      bytes[i+1] = 0;
      printf("Unsupported version string prefix: %s\n", bytes);
      return 0;
    }
  }
  c = (char) get_char(fs, FAIL);
  if (c != '\n') {
    bytes[header_len] = 0;
    fprintf(stderr, "Invalid char following header[%s]: %#4x\n", bytes, c);
    cexit(fs, 1);
  }
  return 1;
}

int is_not_space(int c) {
  return !isspace(c) && c != EOF;
}

int read_bin_comment(FILE* fs) {
  printf("Checking if is bin...\n");
  char c = (char) get_char(fs, 1);
  if (c != '%') {
    unget_char(fs, c, FAIL);
    return 0;
  }

  // arbitrary size. Doesn't seem like there is a limit in 2008 spec.
  const size_t len = 1024;
  unsigned char* chars = consume_chars(fs, &is_not_space, len);

  int i = 0;
  while (chars[i] != 0) {
    printf("%u\n", chars[i]);
    if (chars[i] <= 128) {
      printf("Non-bin char in comment: %#4x\n", chars[i]);
      free(chars);
      return 0;
    }
    ++i;
  }

  free(chars);
  return 1;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }

  FILE* fs = file_exists(argv[1]);
  if (fs) {
    if (version1_7(fs)) {
      int is_bin = read_bin_comment(fs);
      printf("Is %sbin.\n", is_bin ? "" : "not ");
    }
    fclose(fs);
  }

  return 0;
}

