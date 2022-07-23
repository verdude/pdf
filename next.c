#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

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

void unget_chars(FILE* fs, int* c, int len, int fail_on_error) {
  for (int i = 0; i < len; ++i) {
    unget_char(fs, c[i], fail_on_error);
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

void consume_whitespace(FILE* fs) {
  int c;
  do {
    c = get_char(fs, FAIL);
  } while (isspace(c));

  unget_char(fs, c, FAIL);
}

/**
 * Preceding char is '<', check what type of object it could be.
 */
static object_t* next_angle_bracket_sym(FILE* fs) {
  int c = get_char(fs, FAIL);

  switch ((unsigned char) c) {
    case '<':
      return get_list(fs, DictionaryEntry);
    default:
      // hex string
      fprintf(stderr, "Hex string parsing is not yet implemented.\n");
      return NULL;
  }
}

object_t* next_sym(FILE* fs) {
  consume_whitespace(fs);

  int c = get_char(fs, IGNORE);
  if (c == EOF) {
    printf("Reached EOF.");
    return NULL;
  }

  if (isdigit(c)) {
    unget_char(fs, c, FAIL);
    return get_number(fs, FAIL);
  }

  switch ((unsigned char) c) {
    case '/':
      unget_char(fs, c, FAIL);
      return get_name(fs, FAIL);
    case '<':
      return next_angle_bracket_sym(fs);
    case '(':
      unget_char(fs, c, FAIL);
      return get_string(fs, FAIL);
    case '[':
      unget_char(fs, c, FAIL);
      return get_list(fs, Object);
    default:
      fprintf(stderr, "Next char: %i\n", c);
      fprintf(stderr, "unknown symbol! [%c]\n", c);
      return NULL;
  }
}

char* consume_chars(FILE* fs, int (*fn)(), int len) {
  char* chars = allocate(len);
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

/**
 * Checks for a match at the current position.
 * returns the current position if found, 0 otherwise.
 * Returns EOF in case of EOF.
 */
size_t check_for_match(FILE* fs, char* s) {
  if (s[0] == 0) {
    return get_pos(fs);
  }

  int c = get_char(fs, IGNORE);
  if (c == EOF) {
    return EOF;
  }

  if (c != s[0]) {
    return 0;
  }

  return check_for_match(fs, s + 1);
}

// TODO: perhaps validate the sequence? Make sure it is null terminated
// at the given length?
int find_backwards(FILE* fs, char* sequence, int len) {
  if (len > 10) {
    fprintf(stderr, "Sequence too long: %i\n", len);
    return 0;
  }

  long curr_pos = get_pos(fs);
  size_t match = 0;

  while ((match = check_for_match(fs, sequence)) != 1) {
    if (match == EOF) {
      return 0;
    }
    int new_pos = curr_pos - 1 > 0 ? curr_pos - 1 : 0;
    if (new_pos == curr_pos) {
      fprintf(stderr, "Could not find sequence %s in file\n", sequence);
      return 0;
    }
    seek(fs, new_pos, SEEK_SET, FAIL);
    curr_pos = get_pos(fs);
  }

  return 1;
}

void cexit(FILE* fs, int code) {
  fprintf(stderr, "~~~~> exiting with code: %i\n", code);
  if (fs) {
    fclose(fs);
  }
  exit(code);
}

char* fs_read(FILE* fs, size_t size) {
  char* bytes = allocate(size + 1);
  size_t read = fread(bytes, 1, size, fs);

  if (read == size) {
    return bytes;
  }

  fprintf(stderr,
      "fs_read expected to read %li bytes. Read %li instead.\n", size, read);
  if (feof(fs)) {
    fprintf(stderr, "Got EOF before could read bytes");
  } else if (ferror(fs)) {
    perror("fread error:");
  } else {
    fprintf(stderr, "Unknown error from fread.\n");
  }
  cexit(fs, 1);
  return NULL;
}

