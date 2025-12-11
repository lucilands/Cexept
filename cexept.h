#ifndef __CEXEPT_HEADER
#define __CEXEPT_HEADER

#ifndef CEXEPT_STACKTRACE_LEN
#define CEXEPT_STACKTRACE_LEN 5
#endif //CEXEPT_STACKTRACE_LEN
#ifndef CEXEPT_MAX_EXCEPTIONS
#define CEXEPT_MAX_EXCEPTIONS 255
#endif //CEXEPT_MAX_EXCEPTIONS


typedef struct {
  const char *bin;
  const char *func;

  unsigned int offset;
} cexept_stackframe_t;

typedef struct {
  cexept_stackframe_t *frames;
  unsigned int len;
} cexept_stacktrace_t;

typedef struct {
  int code;
  const char *descr;
  
  const char *func;
  const char *file;
  int line;

  cexept_stacktrace_t stacktrace;
} cexception_t;

struct __cexception_frame;

#ifdef CEXEPT_NO_PREFIX
#define THROW(type) CEXEPT_THROW(type)
#define TRY(...) CEXEPT_TRY(__VA_ARGS__)
#define CATCH(__exname__, ...) CEXEPT_CATCH(__exname__, __VA_ARGS__)
#define ERROR_DESCRIPTION(__type, __descr) CEXEPT_ERROR_DESCRIPTION(__type, __descr)
#endif //CEXEPT_NO_PREFIX

#define __CEXEPT_UNIQUE_LABEL2(a, b) a##b
#define __CEXEPT_UNIQUE_LABEL(a, b) __CEXEPT_UNIQUE_LABEL2(a, b)
#define CEXEPT_THROW(type) __cexept_throw((cexception_t) {type, __cexept_descrs[type] ? __cexept_descrs[type] : "Unknown error", __func__, __FILE__, __LINE__, (cexept_stacktrace_t){0}})
#define CEXEPT_TRY(...) do {\
                        struct __cexception_frame __excframe;\
                        __excframe.prev = __cexept_exc_stack;\
                        __excframe.use = 1;\
                        __cexept_exc_stack = &__excframe;\
                        if (setjmp(__excframe.env) == 0) {\
                        __VA_ARGS__
                      
#define CEXEPT_CATCH(__exname__, ...) } else {\
                              cexception_t __exname__ = __cexception;\
                              __VA_ARGS__\
                            }\
                            __cexept_exc_stack = __excframe.prev;\
                        } while (0);

#define CEXEPT_ERROR_DESCRIPTION(__type, __descr) __cexept_descrs[__type] = __descr;


void __cexept_throw(cexception_t exeption);

extern char * __cexept_descrs[CEXEPT_MAX_EXCEPTIONS];
extern cexception_t __cexception;
extern struct __cexception_frame *__cexept_exc_stack;

#ifdef CEXEPT_IMPLEMENTATION
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <execinfo.h>
#include <dlfcn.h>

struct __cexception_frame {
  jmp_buf env;
  struct __cexception_frame *prev;
  _Bool use;
  int code;
};

cexception_t __cexception = {0};
jmp_buf __cexept_jmp_buf = {0};
struct __cexception_frame *__cexept_exc_stack = 0;
char * __cexept_descrs[CEXEPT_MAX_EXCEPTIONS];

cexept_stacktrace_t cexept_backtrace() {
  void **stacktrace = malloc(CEXEPT_STACKTRACE_LEN * sizeof(void*));
  if (!stacktrace) {printf("ERROR: INTERNAL: Failed to allocate memory\n"); exit(1);}

  unsigned int size = backtrace(stacktrace, CEXEPT_STACKTRACE_LEN);
  stacktrace++; size--; // Skip first entry in stacktrace (will always be __cexept_throw)

  cexept_stacktrace_t ret = {
    malloc(size * sizeof(cexept_stackframe_t)),
    size
  };
  if (!ret.frames) {printf("ERROR: INTERNAL: Failed to allocate memory\n"); exit(1);}

  for (unsigned int i = 0; i < size; i++) {
    Dl_info info;
    if (dladdr(stacktrace[i], &info)) {
      ret.frames[i] = (cexept_stackframe_t) {
        info.dli_fname,
        info.dli_sname ? info.dli_sname : "??",

        (unsigned int *)stacktrace[i] - (unsigned int *)info.dli_saddr
      };
    }
    else {
      printf("WARNING: INTERNAL: Failed to fetch stacktrace func\n");
    }
  }
  return ret;
}

void __cexept_throw(cexception_t exception) {
  cexept_stacktrace_t stacktrace = cexept_backtrace();
  exception.stacktrace = stacktrace;

  if (__cexept_exc_stack) {
    if (__cexept_exc_stack->use) {
      __cexception = exception;
      __cexept_exc_stack->use = 0;
      longjmp(__cexept_exc_stack->env, exception.code);
      return;
    }
  }

  printf("stacktrace, most recent call last:\n");
  for (unsigned int i = stacktrace.len-1; i != 0; i--) {
    cexept_stackframe_t frame = stacktrace.frames[i];
    printf("  %s + 0x%x in %s\n", frame.bin, frame.offset, frame.func);
  }
  free(stacktrace.frames);

  printf("%s:%i: in function %s: %s\n", exception.file, exception.line, exception.func, exception.descr);
  exit(exception.code);
}

#endif //CEXEPT_IMPLEMENTATION
#endif //__CEXEPT_HEADER
