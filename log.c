#include <stdio.h>
#include <stdarg.h>

#include "log.h"

static struct {
  char* pre;
  int level;
} log;

void log_quiet() {
  log.level = ERROR;
}

void log_info() {
  log.level = INFO;
}

void log_verbose() {
  log.level = VERBOSE;
}

void log_msg(enum log_level level, char* format, ...) {
  va_list args;
  FILE* stream = level > INFO ? stderr : stdout;

  if (log.level > level) {
    return;
  }

  va_start(args, format);
  vfprintf(stream, format, args);
  va_end(args);

  fprintf(stream, "\n");
}
