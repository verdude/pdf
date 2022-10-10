#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

#include "pdf.h"
#include "next.h"
#include "object.h"
#include "log.h"

int get_char(pdf_t* pdf, int eof_fail) {
  int c = fgetc(pdf->fs);
  if (c == EOF && eof_fail) {
    log_e("Premature EOF.\n");
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

void unget_chars(pdf_t* pdf, unsigned char* s, int len) {
  for (int i = 0; i < len; ++i) {
    unget_char(pdf, s[i], FAIL);
  }
}

void* allocate(int len) {
  void* m = calloc(len, 1);
  if (m == NULL) {
    log_e("Failed to allocate mem of length: %i\n", len);
    perror("allocate");
    exit(1);
  }
  return m;
}

void skip_while(pdf_t* pdf, int (*fn)(int)) {
  int c;
  do {
    c = get_char(pdf, FAIL);
  } while ((*fn)(c));

  unget_char(pdf, c, FAIL);
}

/**
 * Preceding char is '<', check what type of object it could be.
 */
static object_t* next_angle_bracket_sym(pdf_t* pdf) {
  int c = get_char(pdf, FAIL);

  switch ((unsigned char) c) {
    case '<':
      unget_chars(pdf, (unsigned char*) "<<", 2);
      return get_list(pdf, DictionaryEntry);
    default:
      unget_char(pdf, c, FAIL);
      unget_char(pdf, '<', FAIL);
      return get_hex_string(pdf);
  }
}

// rename to next_token or something
char* consume_chars(pdf_t* pdf, int (*fn)(int), int len) {
  char* chars = allocate(len);
  int c;

  for (size_t i = 0; i < len - 1; ++i) {
    c = get_char(pdf, IGNORE);
    if ((*fn)(c)) {
      chars[i] = c;
    } else {
      unget_char(pdf, c, IGNORE);
      break;
    }
  }

  return chars;
}

void consume_chars_stack(pdf_t* pdf, int (*fn)(int), char* chars, int len) {
  int c;
  for (size_t i = 0; i < len - 1; ++i) {
    c = get_char(pdf, IGNORE);
    if ((*fn)(c)) {
      chars[i] = c;
    } else {
      unget_char(pdf, c, IGNORE);
      break;
    }
  }
}

long estrtol(char* s, char** endptr, int base) {
  long n = strtol(s, endptr, base);
  if (n == LONG_MIN || n == LONG_MAX) {
    log_e("strtol failed on input: %s\n", s);
    scexit(NULL, 1);
  }
  return n;
}

void consume_whitespace(pdf_t* pdf) {
  skip_while(pdf, &isspace);
}

int skip_string(pdf_t* pdf, char* s, long pos) {
  if (*s == 0) {
    return 1;
  }
  int c = get_char(pdf, FAIL);

  if (*s != (char) c) {
    seek(pdf, pos, SEEK_SET);
    return 0;
  }

  return skip_string(pdf, s+1, pos);
}

int is_not_space(int c) {
  return !isspace(c) && c != EOF;
}

object_t* next_sym(pdf_t* pdf) {
  consume_whitespace(pdf);

  int c = get_char(pdf, FAIL);
  if (c == EOF) {
    log_v("Reached EOF.");
    return NULL;
  }

  if (isdigit(c)) {
    unget_char(pdf, c, FAIL);
    return parse_num(pdf);
  }

  switch ((unsigned char) c) {
    case '/':
      unget_char(pdf, c, FAIL);
      return get_name(pdf, FAIL);
    case '<':
      return next_angle_bracket_sym(pdf);
    case '(':
      unget_char(pdf, c, FAIL);
      return get_string(pdf);
    case '[':
      unget_char(pdf, c, FAIL);
      return get_list(pdf, Object);
    case 'n':
      unget_char(pdf, c, FAIL);
      return get_term(pdf, NullTerm);
    case 't':
      unget_char(pdf, c, FAIL);
      return get_term(pdf, TrueTerm);
    case 'f':
      unget_char(pdf, c, FAIL);
      return get_term(pdf, FalseTerm);
    case '-':
      long n = get_num(pdf, 10, FAIL);
      return create_num_obj(pdf, get_pos(pdf), -n);
    default:
      log_e("next_sym: unknown symbol! [%c] int: %i\n", c, c);
      log_e("pos: %li\n", get_pos(pdf)-1);
      return NULL;
  }
}

long get_pos(pdf_t* pdf) {
  long pos = ftell(pdf->fs);
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
size_t check_for_match(pdf_t* pdf, char* s) {
  if (*s == 0) {
    return get_pos(pdf);
  }

  int c = get_char(pdf, FAIL);

  if (c != *s) {
    return 0;
  }

  return check_for_match(pdf, s + 1);
}

size_t check_for_match_seek_back(pdf_t* pdf, char* s) {
  long pos = get_pos(pdf);
  size_t result = check_for_match(pdf, s);
  seek(pdf, pos, SEEK_SET);
  return result;
}

// TODO: perhaps validate the sequence? Make sure it is null terminated
// at the given length?
int find_backwards(pdf_t* pdf, char* sequence, int len) {
  if (len > 15) {
    log_e("Sequence too long: %i\n", len);
    return 0;
  }

  long curr_pos = get_pos(pdf);
  size_t match = 0;

  while ((match = check_for_match(pdf, sequence)) <= 0) {
    if (match == EOF) {
      return 0;
    }
    int new_pos = curr_pos - 1 > 0 ? curr_pos - 1 : 0;
    if (new_pos == curr_pos) {
      log_e("Could not find sequence %s in file\n", sequence);
      return 0;
    }
    seek(pdf, new_pos, SEEK_SET);
    curr_pos = new_pos;
  }

  return 1;
}

void scexit(pdf_t* pdf, int code) {
  void *array[10];
  size_t size;

  if (pdf) {
    log_e("~~~~> Offset: %li\n", ftell(pdf->fs));
  } else {
    log_e("~~~~> PDF/File Stream is NULL.\n");
  }

  log_e("~~~~> Exiting with code: %i\n", code);

  size = backtrace(array, 10);
  backtrace_symbols_fd(array, size, 1);

  free_pdf_t(pdf);

  exit(code);
}

unsigned char* fs_read(pdf_t* pdf, size_t size) {
  unsigned char* bytes = allocate(size);
  size_t read = fread(bytes, 1, size, pdf->fs);

  if (read == size) {
    return bytes;
  }

  log_e("fs_read expected to read %li bytes. Read %li instead.\n", size, read);

  if (feof(pdf->fs)) {
    log_e("Got EOF before could read bytes");
  } else if (ferror(pdf->fs)) {
    perror("fread error:");
  } else {
    log_e("Unknown error from fread.\n");
  }
  scexit(pdf, 1);
  // Return just to make the compiler happy
  return NULL;
}
