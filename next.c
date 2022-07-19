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

  printf("Consumed %li chars out of possible %i.\n",
      strnlen((char*) chars, len), len);
  return chars;
}

long get_pos(FILE* fs) {
  long pos = ftell(fs);
  if (pos == -1) {
    perror("ftell");
    exit(1);
  }
  return pos;
}

int seek(FILE* fs, long offset, int whence, int fail_on_error) {
  int ret = fseek(fs, offset, whence);
  if (ret == -1 && fail_on_error) {
    perror("fseek");
    cexit(fs, 1);
  }
  return ret;
}

// Checks for a match at the current position.
static long check_for_match(FILE* fs, unsigned char* s) {
  if (s[0] == 0) {
    return 1;
  }

  int c = get_char(fs, IGNORE);
  if (c == EOF) {
    return EOF;
  }

  if ((unsigned char) c != s[0]) {
    unget_char(fs, c, FAIL);
    return 0;
  }

  return check_for_match(fs, s + 1);
}

int find_backwards(FILE* fs, unsigned char* sequence, int len) {
  if (len > 10) {
    fprintf(stderr, "Sequence too long: %i\n", len);
    return -1;
  }

  long curr_pos = get_pos(fs);
  int match = 0;
  while ((match = check_for_match(fs, sequence)) != 1) {
    if (match == EOF) {
      return -1;
    }
    seek(fs, curr_pos - 1 > 0 ? curr_pos - 1 : 0, SEEK_SET, FAIL);
    curr_pos = get_pos(fs);
  }
  return curr_pos;
}

void cexit(FILE* fs, int code) {
  fclose(fs);
  exit(code);
}

