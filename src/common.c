#include "common.h"


void panic_(const char *str, int line, const char *file, const char *func) {
  fprintf(stderr, "fatal error: %s:%d in %s(): %s\n", file, line, func, str);
  exit(EXIT_FAILURE);
}


void expect_(const char *str, int line, const char *file, const char *func) {
  char buf[1024];
  sprintf(buf, "assertion failed: \"%s\"", str);
  panic_(buf, line, file, func);
}


int string_to_enum(const char **strings, const char *str) {
  for (int i = 0; strings[i]; i++) {
    if (strcmp(strings[i], str) == 0) {
      return i;
    }
  }
  return -1;
}


bool string_equal_nocase(const char *a, const char *b) {
  while (*a) {
    if (tolower(*a) != tolower(*b)) {
      return false;
    }
    a++; b++;
  }
  return *a == *b;
}


bool string_is_empty(const char *str) {
  while (*str) {
    if (!isspace(*str)) { return false; }
    str++;
  }
  return true;
}
