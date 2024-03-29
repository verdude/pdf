#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "next.h"
#include "trailer.h"
#include "xref.h"
#include "log.h"

FILE* file_exists(char* path) {
  FILE* fs = fopen(path, "r");
  if (fs) {
    return fs;
  }

  perror(path);
  return 0;
}

void free_pdf_t(pdf_t* pdf) {
  if (!pdf) {
    return;
  }

  free_trailer_t(pdf->trailer);
  free_xref_t(pdf->xref);
  if (pdf->fs) {
    fclose(pdf->fs);
  }
}

int supported_version(pdf_t* pdf) {
  const int prefix_len = 5;
  const int version_len = 3;
  const int header_len = prefix_len + version_len;
  const char* prefix = "%PDF-";
  const char version[3][6] = {"1", ".", "34576"};
  char bytes[header_len+1];
  char c;

  for (size_t i = 0; i < prefix_len; ++i) {
    c = get_char(pdf, FAIL);
    bytes[i] = (char) c;

    if (c != prefix[i]) {
      bytes[i+1] = 0;
      log_v("Unsupported prefix: %s, Should prefix(v): %s", bytes, prefix);
      return 0;
    }
  }

  for (size_t i = 0; i < version_len; ++i) {
    int ok = 0;
    c = get_char(pdf, FAIL);
    bytes[i] = (char) c;

    for (size_t j = 0; j < strlen(version[i]); ++j) {
      if (c == version[i][j]) {
        ok = 1;
        break;
      }
    }

    if (!ok) {
      bytes[i+1] = 0;
      log_v("Unsupported version string prefix: %s", bytes);
      return 0;
    }
  }

  c = (char) get_char(pdf, FAIL);
  if (c != '\n') {
    bytes[header_len] = 0;
    log_e("Invalid char following header[%s]: %#4x", bytes, c);
    scexit(pdf, 1);
  }
  return 1;
}

int read_bin_comment(pdf_t* pdf) {
  char c = (char) get_char(pdf, FAIL);
  if (c != '%') {
    unget_char(pdf, c, FAIL);
    return 0;
  }

  // arbitrary size. Doesn't seem like there is a limit in 2008 spec.
  const size_t len = 1024;
  char chars[1024] = {0};
  consume_chars_stack(pdf, &is_not_space, chars, len);

  int i = 0;
  while (chars[i] != 0) {
    if ((unsigned char) chars[i] <= 128) {
      log_v("Non-bin char in comment: %#4x", chars[i]);
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

  int opt;

  log_info();

  while((opt = getopt(argc, argv, ":ve")) != -1) {
    switch (opt) {
      case 'v':
        log_verbose();
        break;
      case 'e':
        log_quiet();
        break;
      default:
        log_i("umm, haha...");
        break;
    }
  }

  if (optind >= argc) {
    log_e("missing filename.");
    return 1;
  }

  FILE* fs = file_exists(argv[optind]);
  pdf_t* pdf = allocate(sizeof(pdf));
  if (fs) {
    pdf->fs = fs;
    if (supported_version(pdf)) {
      log_v("Getting trailer...");
      int success = get_trailer(pdf);
      if (!success) {
        log_e("failed to get trailer");
        scexit(pdf, 1);
      }
      log_v("Getting xref table");
      success = get_xref(pdf);
      if (success) {
        log_v("Getting entries");
        parse_entries(pdf);
      }
      free_pdf_t(pdf);
    }
  }

  return 0;
}
