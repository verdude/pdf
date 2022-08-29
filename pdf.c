#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "next.h"
#include "trailer.h"
#include "xref.h"

FILE* file_exists(char* path) {
  FILE* fs = fopen(path, "r");
  if (fs) {
    return fs;
  }

  perror(path);
  return 0;
}

int supported_version(FILE* fs) {
  const int prefix_len = 5;
  const int version_len = 3;
  const int header_len = prefix_len + version_len;
  const char* prefix = "%PDF-";
  const char version[3][3] = {"1", ".", "76"};
  char bytes[header_len+1];
  char c;

  for (size_t i = 0; i < prefix_len; ++i) {
    c = get_char(fs, FAIL);
    bytes[i] = (char) c;

    if (c != prefix[i]) {
      bytes[i+1] = 0;
      printf("Unsupported prefix: %s, Should prefix(v): %s\n", bytes, prefix);
      return 0;
    }
  }

  for (size_t i = 0; i < version_len; ++i) {
    int ok = 0;
    c = get_char(fs, FAIL);
    bytes[i] = (char) c;

    for (size_t j = 0; j < strlen(version[i]); ++j) {
      if (c == version[i][j]) {
        ok = 1;
        break;
      }
    }

    if (!ok) {
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

int read_bin_comment(FILE* fs) {
  char c = (char) get_char(fs, FAIL);
  if (c != '%') {
    unget_char(fs, c, FAIL);
    return 0;
  }

  // arbitrary size. Doesn't seem like there is a limit in 2008 spec.
  const size_t len = 1024;
  char chars[1024] = {0};
  consume_chars_stack(fs, &is_not_space, chars, len);

  int i = 0;
  while (chars[i] != 0) {
    if ((unsigned char) chars[i] <= 128) {
      printf("Non-bin char in comment: %#4x\n", chars[i]);
      return 0;
    }
    ++i;
  }

  return 1;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }

  FILE* fs = file_exists(argv[1]);
  if (fs) {
    if (supported_version(fs)) {
      trailer_t* trailer = get_trailer(fs);
      if (!trailer) {
        cexit(fs, 1);
      }
      xref_t* xref = get_xref(fs, trailer->startxref_offset);
      if (trailer) {
        print_object(trailer->dictionary);
        free_trailer_t(trailer);
      }
      if (xref) {
        parse_entries(fs, xref);
        free_xref_t(xref);
      }
    }
    fclose(fs);
  }

  return 0;
}

