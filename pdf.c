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
  char c;
  for (size_t i = 0; i < header_len; ++i) {
    c = fgetc(fs);
    if (c != header[i]) {
      printf("umm, weird...\n");
      return 0;
    }
  }
  if (fseek(fs, header_len + 1, SEEK_SET) == -1) {
    perror("fseek past header");
    exit(1);
  }
  return 1;
}

int is_not_space(int c) {
  return !isspace(c);
}

int read_bin_comment(FILE* fs) {
  printf("Checking if is bin...\n");
  char c = (char) get_char(fs, 1);
  if (c != '%') {
    return 0;
  }
  // arbitrary size. Doesn't seem like there is a limit in 2008 spec.
  const size_t len = 1024;
  char* chars = consume_chars(fs, &is_not_space, len);

  int i = 0;
  while (chars[i] != 0) {
    printf("%u\n", (unsigned char) chars[i]);
    if (chars[i] <= 128) {
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
      read_bin_comment(fs);
    }
    fclose(fs);
  }

  return 0;
}

