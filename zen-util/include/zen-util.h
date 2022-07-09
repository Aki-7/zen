#ifndef ZEN_UTIL_H
#define ZEN_UTIL_H

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/** Visibility attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define ZN_EXPORT __attribute__((visibility("default")))
#else
#define ZN_EXPORT
#endif

/** Compile-time computation of number of items in an array */
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a)[0])
#endif

/** Suppress compiler warnings for unused variables */
#ifndef UNUSED
#define UNUSED(x) ((void)x)
#endif

/** Allocate memory and set to zero */
static inline void *
zalloc(size_t size)
{
  return calloc(1, size);
}

/** Retrieve a pointer to a containing struct */
#define zn_container_of(ptr, sample, member) \
  (__typeof__(sample))((char *)(ptr)-offsetof(__typeof__(*sample), member))

/** logger */
int zn_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* alternative logger api */
int zn_vlog(const char *fmt, va_list ap);

/* convert string to integer */
static inline bool
zn_safe_strtoint(const char *str, int32_t *value)
{
  long ret;
  char *end;

  assert(str != NULL);

  errno = 0;
  ret = strtol(str, &end, 10);
  if (errno != 0)
    return false;
  else if (end == str || *end != '\0') {
    errno = EINVAL;
    return false;
  }

  if ((long)((int32_t)ret) != ret) {
    errno = ERANGE;
    return false;
  }

  *value = (int32_t)ret;

  return true;
}

#endif  //  ZEN_UTIL_H
