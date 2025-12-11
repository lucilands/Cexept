#ifndef __CEXEPT_HEADER
#define __CEXEPT_HEADER

#include <setjmp.h>

#ifndef CEXEPT_STACKTRACE_LEN
#define CEXEPT_STACKTRACE_LEN 5
#endif

typedef struct {
  int code;
  const char *descr;
  
  const char *func;
  const char *file;
  int line;
} cexception_t;

struct __cexception_frame {
  jmp_buf env;
  struct __cexception_frame *prev;
  int code;
};

#ifdef CEXEPT_NO_PREFIX
#define THROW(type) CEXEPT_THROW(type)
#define TRY(...) CEXEPT_TRY(__VA_ARGS__)
#define CATCH(__exname__, ...) CEXEPT_CATCH(__exname__, __VA_ARGS__)
#endif //CEXEPT_NO_PREFIX

#define __CEXEPT_UNIQUE_LABEL2(a, b) a##b
#define __CEXEPT_UNIQUE_LABEL(a, b) __CEXEPT_UNIQUE_LABEL2(a, b)
#define CEXEPT_THROW(type) __cexept_throw((cexception_t) {type, __cexet_get_str(type), __func__, __FILE__, __LINE__})
#define CEXEPT_TRY(...) \
                        __cexept_try = 1;\
                        struct __cexception_frame __excframe;\
                        __excframe.prev = __cexept_exc_stack;\
                        __cexept_exc_stack = &__excframe;\
                        if (setjmp(__excframe.env) == 0) {\
                        __VA_ARGS__\
                        
#define CEXEPT_CATCH(__exname__, ...) } else {\
                            cexception_t __exname__ = __cexception;\
                            __cexept_try = 0;\
                            __VA_ARGS__\
                            }\


void __cexept_throw(cexception_t exeption);
const char *__cexet_get_str(int code);

extern int __cexept_try;
extern int __cexept_triggered;
extern cexception_t __cexception;
extern struct __cexception_frame *__cexept_exc_stack;

#ifdef CEXEPT_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include <execinfo.h>

int __cexept_try = 0;
int __cexept_triggered = 0;
cexception_t __cexception = {0};
jmp_buf __cexept_jmp_buf = {0};
struct __cexception_frame *__cexept_exc_stack = 0;

const char *__cexet_get_str(int code) {
  switch (code) {
    default: return "Unknown exeption";
  }
}

void __cexept_throw(cexception_t exception) {
  if (__cexept_try) {
    __cexception = exception;
    __cexept_triggered = 1;
    longjmp(__cexept_exc_stack->env, exception.code);
    return;
  }

  printf("stacktrace, most recent call last:\n");
  void **stacktrace = malloc(CEXEPT_STACKTRACE_LEN * sizeof(void*));
  if (!stacktrace) {printf("ERROR: INTERNAL: Failed to allocate memory\n"); exit(1);}

  int size = backtrace(stacktrace, CEXEPT_STACKTRACE_LEN);
  char **strings = backtrace_symbols(stacktrace, size);
  if (strings) {
    for (int i = size-1; i > 0; i--) {
      printf("%s\n", strings[i]);
    }
  }
  free(strings);

  printf("%s:%i: in function %s: %s\n", exception.file, exception.line, exception.func, __cexet_get_str(exception.code));

  exit(exception.code);
}

#endif //CEXEPT_IMPLEMENTATION
#endif //__CEXEPT_HEADER
