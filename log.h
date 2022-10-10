#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

enum log_level {
  VERBOSE,
  INFO,
  ERROR,
};

void log_quiet();
void log_level(enum log_level level);
void log_msg(enum log_level level, char* format, ...);

#define log_v(...) log_msg(VERBOSE, __VA_ARGS__)
#define log_i(...) log_msg(INFO, __VA_ARGS__)
#define log_e(...) log_msg(ERROR, __VA_ARGS__)

#endif // LOG_H
