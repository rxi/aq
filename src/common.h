#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define panic(x) panic_(x, __LINE__, __FILE__, __func__)
#define expect(x) do { if (!(x)) { expect_(#x, __LINE__, __FILE__, __func__); }} while(0);

static inline float clampf(float n, float lo, float hi) { return n < lo ? lo : n > hi ? hi : n; }
static inline float minf(float a, float b) { return a < b ? a : b; }
static inline float maxf(float a, float b) { return a > b ? a : b; }
static inline float lerpf(float a, float b, float p) { return a + (b - a) * p; }

void panic_(const char *str, int line, const char *file, const char *func);
void expect_(const char *str, int line, const char *file, const char *func);
int string_to_enum(const char **strings, const char *str);
bool string_equal_nocase(const char *a, const char *b);
bool string_is_empty(const char *str);

#endif
