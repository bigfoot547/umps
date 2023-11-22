#ifndef UMPS_MACROS_H_INCLUDED
#define UMPS_MACROS_H_INCLUDED

/* define handy macros (unreachable and trap) */

#ifdef __GNUC__ /* gcc and clang */

/* unreachable statement: umps_unreachable; */
#define umps_unreachable __builtin_unreachable()

#ifdef NDEBUG

/* assume a trap will never be hit in release */
#define umps_trap umps_unreachable

#else

#include <stdlib.h> /* for abort() (guaranteed noreturn) */
#define umps_trap do { __builtin_trap(); abort(); } while(0)

#endif /* defined(NDEBUG) */

#else /* !defined(__GNUC__) */

#include <stdlib.h> /* for abort() */
#define umps_unreachable umps_trap
#define umps_trap abort()

#endif /* defined(__GNUC__) */

/* define assert macros */

#ifdef NDEBUG
/* asserts do nothing in release builds (arguments could have side effects) */
#define umps_assert(_e) ((void)(_e))
#define umps_assert_s(_e, _s) (((void)(_e), (void)(_s)))
#else

#include <stdio.h>  /* for fprintf, fflush, stderr */

#define umps_assert(_e) do { \
  if (!(_e)) {               \
    fprintf(stderr, "!!!! UMPS assertion failed: %s:%d %s %s\n", \
        __FILE__, __LINE__, __func__, #_e); \
    fflush(stderr);          \
    umps_trap;               \
  }                          \
} while(0)

#define umps_assert_s(_e, _s) do { \
  if (!(_e)) {                     \
    fprintf(stderr, "!!!! UMPS assertion failed (%s): %s:%d %s %s\n", \
        _s, __FILE__, __LINE__, __func__, #_e); \
    fflush(stderr);                \
    umps_trap;                     \
  }                                \
} while(0)

#endif /* defined(NDEBUG) */

#endif /* include guard */
