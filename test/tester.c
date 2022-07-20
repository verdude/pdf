#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define NAME_MAX 255
#define EXT_LEN 4
#define EXT ".pdf"

int ends_with(char* s, char* e, int elen) {
  if (elen <= 0) {
    return 1;
  }

  if (*s != *e) {
    return 0;
  }

  return ends_with(--s, --e, --elen);
}

int main(int argc, char** argv) {
  DIR* d = opendir("assets");
  struct dirent* dir;

  if (d) {
    while ((dir = readdir(d)) != NULL) {
      size_t name_len = strnlen(dir->d_name, NAME_MAX);
      if (name_len < EXT_LEN) {
        printf("Skipping: %s\n", dir->d_name);
        continue;
      }
      if (ends_with(dir->d_name + name_len - 1, EXT + EXT_LEN - 1, EXT_LEN)) {
        printf("Found [%s]: %s\n", EXT, dir->d_name);
      }
    }
    closedir(d);
  }
  return 0;
}
