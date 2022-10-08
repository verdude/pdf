#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

#include "pdf.h"
#include "next.h"
#include "object.h"

int get_char(pdf_t* pdf, int eof_fail) {
  int c = fgetc(pdf->fs);
  if (c == EOF && eof_fail) {
    fprintf(stderr, "Premature EOF.\n");
    perror("get_char");
    scexit(pdf, 1);
  }
  return c;
}

void unget_char(pdf_t* pdf, int c, int fail_on_error) {
  clearerr(pdf->fs);
  int r = ungetc(c, pdf->fs);
  if (r == EOF && fail_on_error) {
    perror("unget_char failure");
    scexit(pdf, 1);
  }
}

void unget_chars(FILE* fs, unsigned char* s, int len) {
  for (int i = 0; i < len; ++i) {
    unget_char(fs, s[i], FAIL);
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

void skip_while(FILE* fs, int (*fn)(int)) {
  int c;
  do {
    c = get_char(fs, FAIL);
  } while ((*fn)(c));

  unget_char(fs, c, FAIL);
}

/**
 * Preceding char is '<', check what type of object it could be.
 */
static object_t* next_angle_bracket_sym(pdf_t* pdf) {
  int c = get_char(pdf->fs, FAIL);

  switch ((unsigned char) c) {
    case '<':
      unget_chars(pdf->fs, (unsigned char*) "<<", 2);
      return get_list(pdf, DictionaryEntry);
    default:
      unget_char(pdf->fs, c, FAIL);
      unget_char(pdf->fs, '<', FAIL);
      return get_hex_string(pdf);
  }
}

// rename to next_token or something
char* consume_chars(FILE* fs, int (*fn)(int), int len) {
  char* chars = allocate(len);
  int c;

  for (size_t i = 0; i < len - 1; ++i) {
    c = get_char(fs, IGNORE);
    if ((*fn)(c)) {
      chars[i] = c;
    } else {
      unget_char(fs, c, IGNORE);
      break;
    }
  }

  return chars;
}

void consume_chars_stack(FILE* fs, int (*fn)(int), char* chars, int len) {
  int c;
  for (size_t i = 0; i < len - 1; ++i) {
    c = get_char(fs, IGNORE);
    if ((*fn)(c)) {
      chars[i] = c;
    } else {
      unget_char(fs, c, IGNORE);
      break;
    }
  }
}

long estrtol(char* s, char** endptr, int base) {
  long n = strtol(s, endptr, base);
  if (n == LONG_MIN || n == LONG_MAX) {
    fprintf(stderr, "strtol failed on input: %s\n", s);
    scexit(NULL, 1);
  }
  return n;
}

void consume_whitespace(FILE* fs) {
  skip_while(fs, &isspace);
}

int skip_string(FILE* fs, char* s, long pos) {
  if (*s == 0) {
    return 1;
  }
  int c = get_char(fs, FAIL);

  if (*s != (char) c) {
    seek(fs, pos, SEEK_SET);
    return 0;
  }

  return skip_string(fs, s+1, pos);
}

int is_not_space(int c) {
  return !isspace(c) && c != EOF;
}

object_t* next_sym(pdf_t* pdf) {
  consume_whitespace(pdf->fs);

  int c = get_char(pdf->fs, IGNORE);
  if (c == EOF) {
    printf("Reached EOF.");
    return NULL;
  }

  if (isdigit(c)) {
    unget_char(pdf->fs, c, FAIL);
    return parse_num(pdf);
  }

  switch ((unsigned char) c) {
    case '/':
      unget_char(pdf->fs, c, FAIL);
      return get_name(pdf, FAIL);
    case '<':
      return next_angle_bracket_sym(pdf);
    case '(':
      unget_char(pdf->fs, c, FAIL);
      return get_string(pdf);
    case '[':
      unget_char(pdf->fs, c, FAIL);
      return get_list(pdf, Object);
    case 'n':
      unget_char(pdf->fs, c, FAIL);
      return get_term(pdf, NullTerm);
    case 't':
      unget_char(pdf->fs, c, FAIL);
      return get_term(pdf, TrueTerm);
    case 'f':
      unget_char(pdf->fs, c, FAIL);
      return get_term(pdf, FalseTerm);
    case '-':
      long n = get_num(pdf, 10, FAIL);
      return create_num_obj(pdf, get_pos(pdf->fs) - 1, -n);
    default:
      fprintf(stderr, "next_sym: unknown symbol! [%c] int: %i\n", c, c);
      fprintf(stderr, "pos: %li\n", get_pos(pdf->fs)-1);
      return NULL;
  }
}

long get_pos(FILE* fs) {
  long pos = ftell(fs);
  if (pos == -1) {
    perror("ftell");
    exit(1);
  }
  return pos;
}

int seek(pdf_t* pdf, long offset, int whence) {
  int ret = fseek(pdf->fs, offset, whence);
  if (ret == -1) {
    perror("fseek");
    scexit(pdf, 1);
  }
  return ret;
}

/**
 * Checks for a match at the current position.
 * returns the current position if found, 0 otherwise.
 * Exits in case of EOF.
 * Are size_t and EOF compatible?
 */
size_t check_for_match(FILE* fs, char* s) {
  if (*s == 0) {
    return get_pos(fs);
  }

  int c = get_char(fs, FAIL);

  if (c != *s) {
    return 0;
  }

  return check_for_match(fs, s + 1);
}

size_t check_for_match_seek_back(FILE* fs, char* s) {
  long pos = get_pos(fs);
  size_t result = check_for_match(fs, s);
  seek(fs, pos, SEEK_SET);
  return result;
}

// TODO: perhaps validate the sequence? Make sure it is null terminated
// at the given length?
int find_backwards(FILE* fs, char* sequence, int len) {
  if (len > 15) {
    fprintf(stderr, "Sequence too long: %i\n", len);
    return 0;
  }

  long curr_pos = get_pos(fs);
  size_t match = 0;

  while ((match = check_for_match(fs, sequence)) <= 0) {
    if (match == EOF) {
      return 0;
    }
    int new_pos = curr_pos - 1 > 0 ? curr_pos - 1 : 0;
    if (new_pos == curr_pos) {
      fprintf(stderr, "Could not find sequence %s in file\n", sequence);
      return 0;
    }
    seek(fs, new_pos, SEEK_SET);
    curr_pos = new_pos;
  }

  return 1;
}

void scexit(pdf_t* pdf, int code) {
  void *array[10];
  size_t size;

  if (pdf->fs) {
    fprintf(stderr, "~~~~> Offset: %li\n", ftell(fs));
  } else {
    fprintf(stderr, "~~~~> File Stream is NULL.\n");
  }

  fprintf(stderr, "~~~~> Exiting with code: %i\n", code);

  size = backtrace(array, 10);
  backtrace_symbols_fd(array, size, 1);

  free_pdf_t(pdf);

  exit(code);
}

unsigned char* fs_read(FILE* fs, size_t size) {
  unsigned char* bytes = allocate(size);
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
  scexit(pdf, 1);
  // Return just to make the compiler happy
  return NULL;
}
