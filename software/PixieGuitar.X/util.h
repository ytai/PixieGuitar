#ifndef UTIL_H
#define	UTIL_H

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

static inline int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

static inline int limit(int a, int min, int max) {
  assert(max >= min);
  if (a < min) return min;
  if (a > max) return max;
  return a;
}

#endif  // UTIL_H

