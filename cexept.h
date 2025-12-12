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
#define CEXEPT_TRY(...) {\
                        struct __cexception_frame __excframe;\
                        __excframe.prev = __cexept_exc_stack;\
                        __excframe.use = 1;\
                        __cexept_exc_stack = &__excframe;\
                        if (setjmp(__excframe.env) == 0) {\
                        __VA_ARGS__
                      
#define CEXEPT_CATCH(__exname__, ...) \
                            } else {\
                              cexception_t __exname__ = __cexception;\
                              __VA_ARGS__\
                            }\
                            __cexept_exc_stack = __excframe.prev;\
                        };

#define CEXEPT_ERROR_DESCRIPTION(__type, __descr) __cexept_descrs[__type] = __descr;
#define CEXEPT_FREE_STACKTRACE(stacktrace) for (unsigned int i = 0; i < (stacktrace.len); i++) {free((char*)(stacktrace).frames[i].func);free((char*)(stacktrace).frames[i].bin);}\
                                           free(stacktrace.frames);


void __cexept_throw(cexception_t exeption);

extern char * __cexept_descrs[CEXEPT_MAX_EXCEPTIONS];
extern cexception_t __cexception;
extern struct __cexception_frame *__cexept_exc_stack;

#ifdef CEXEPT_IMPLEMENTATION
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#ifdef __unix__
#include <dlfcn.h>
#include <execinfo.h>
#include <string.h>
#endif //__unix__
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#endif //_WIN32

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

#ifdef __unix__
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
        strdup(info.dli_fname),
        info.dli_sname ? strdup(info.dli_sname) : strdup("??"),

        (unsigned int *)stacktrace[i] - (unsigned int *)info.dli_saddr
      };
    }
    else {
      printf("WARNING: INTERNAL: Failed to fetch stacktrace func\n");
    }
  }
  return ret;
}
#endif //__unix__
#ifdef _WIN32
static void resolve(void *addr, cexept_stackframe_t *f) {
    HANDLE p = GetCurrentProcess();
    SymInitialize(p, NULL, TRUE);

    DWORD64 base = SymGetModuleBase64(p, (DWORD64)addr);
    char modbuf[MAX_PATH];
    if (base && GetModuleFileNameA((HMODULE)base, modbuf, MAX_PATH))
        f->bin = _strdup(modbuf);
    else
        f->bin = _strdup("??");

    BYTE buf[sizeof(SYMBOL_INFO) + 256];
    PSYMBOL_INFO s = (PSYMBOL_INFO)buf;
    s->SizeOfStruct = sizeof(SYMBOL_INFO);
    s->MaxNameLen = 255;

    if (SymFromAddr(p, (DWORD64)addr, NULL, s)) {
        f->func = _strdup(s->Name);
        f->offset = (unsigned int)((DWORD64)addr - s->Address);
    } else {
        f->func = _strdup("??");
        f->offset = 0;
    }
}

cexept_stacktrace_t cexept_backtrace() {
    enum { N = 64 };
    void **buf = malloc(N * sizeof(void*));
    unsigned short n = RtlCaptureStackBackTrace(0, N, buf, NULL);

    if (n <= 1) {
        free(buf);
        return (cexept_stacktrace_t){ NULL, 0 };
    }

    void **stk = buf + 1;
    n--;

    cexept_stacktrace_t out = {
        malloc(n * sizeof(cexept_stackframe_t)),
        n
    };

    for (unsigned int i = 0; i < n; i++)
        resolve(stk[i], &out.frames[i]);

    return out;
}
#endif //_WIN32

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
  CEXEPT_FREE_STACKTRACE(stacktrace);
  //free(stacktrace.frames);

  printf("%s:%i: in function %s: %s\n", exception.file, exception.line, exception.func, exception.descr);
  exit(exception.code);
}

#endif //CEXEPT_IMPLEMENTATION
#endif //__CEXEPT_HEADER
