#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define NAME_MAX 255
#define EXT_LEN 4
#define EXT ".pdf"
#define PDF_DIR "assets/"

int ends_with(char* s, char* e, int elen) {
  if (elen <= 0) {
    return 1;
  }

  if (*s != *e) {
    return 0;
  }

  return ends_with(--s, --e, --elen);
}

int run_pdf(char* name) {
  int pid = fork();
  if (pid == -1) {
    perror("fork");
    return 0;
  }

  if (pid) {
    waitpid(pid, NULL, 0);
    return 1;
  } else {
    char* test_runner = "test/tester";
    char* argv[] = { test_runner, name, NULL };
    execv("build/pdf", argv);
    perror(name);
    exit(1);
  }
}

int main(int argc, char** argv) {
  char* assets = PDF_DIR;
  DIR* d = opendir(assets);
  struct dirent* dir;

  if (d) {
    while ((dir = readdir(d)) != NULL) {
      size_t name_len = strnlen(dir->d_name, NAME_MAX);
      if (name_len < EXT_LEN) {
        continue;
      }
      if (ends_with(dir->d_name + name_len - 1, EXT + EXT_LEN - 1, EXT_LEN)) {
        printf("Running pdf on: [%s]: %s\n", EXT, dir->d_name);
        char* filepath = calloc(1, strlen(assets) + strlen(dir->d_name) + 1);
        strcpy(filepath, assets);
        strcpy(filepath+strlen(assets), dir->d_name);
        run_pdf(filepath);
        free(filepath);
      }
    }
    closedir(d);
  }
  return 0;
}
