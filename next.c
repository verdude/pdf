#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"

int get_char(FILE* fs, int eof_fail) {
  int c = fgetc(fs);
  if (c == EOF && eof_fail) {
    fprintf(stderr, "Premature EOF.\n");
    perror("get_char");
    cexit(fs, 1);
  }
  return c;
}

void unget_char(FILE* fs, int c, int fail_on_error) {
  clearerr(fs);
  int r = ungetc(c, fs);
  if (r == EOF && fail_on_error) {
    perror("unget_char failure");
    cexit(fs, 1);
  }
}

void* allocate(int len) {
  void* m = calloc(len, 1);
  if (m == NULL) {
    fprintf(stderr, "Failed to allocate mem of length: %i\n", len);
    perror("allocate");
    exit(1);
  }
  return m;
}

static void consume_whitespace(FILE* fs) {
  int c;
  do {
    c = get_char(fs, FAIL);
  } while (isspace(c));

  unget_char(fs, c, FAIL);
}

char* next_sym(FILE* fs) {
  consume_whitespace(fs);

  int c = get_char(fs, IGNORE);
  if (c == EOF) {
    printf("Reached EOF.");
    return NULL;
  }

  printf("Got next char: %i\n", c);

  return NULL;
}

unsigned char* consume_chars(FILE* fs, int (*fn)(), int len) {
  unsigned char* chars = allocate(len);
  int c;

  for (size_t i = 0; i < len - 1; ++i) {
    c = get_char(fs, IGNORE);
    if (fn(c)) {
      chars[i] = (unsigned char) c;
    } else {
      unget_char(fs, c, IGNORE);
      break;
    }
  }

  printf("Consumed %li chars out of possible %i.\n", strnlen((char*) chars, len), len);
  return chars;
}

int seek(FILE* fs, long offset, int whence, int fail_on_error) {
  int pos = fseek(fs, offset, whence);
  if (pos == -1 && fail_on_error) {
    perror("fseek");
    cexit(fs, 1);
  }
  return pos;
}

void cexit(FILE* fs, int code) {
  fclose(fs);
  exit(code);
}

