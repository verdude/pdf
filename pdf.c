#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
  }
  return 1;
}

int read_bin_comment(FILE* fs) {
  const size_t len = 1024;
  char* chars = malloc(len);
  memset(chars, 0, len);
  if (fgets(chars, len, fs) == NULL) {
    perror("Reading Bin Comment:");
    exit(1);
  }
  int i = 0;
  while (chars[i] != 0) {
    printf("%u\n", (unsigned char) chars[i]);
    ++i;
  }
  free(chars);
  return 0;
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
