#define CEXEPT_IMPLEMENTATION
#define CEXEPT_NO_PREFIX
#include "cexept.h"


#define SOME_ERROR 69

void other_func() {
  printf("before error\n");
  THROW(SOME_ERROR);
  printf("after error\n");
}

void some_func() {
  TRY(
    other_func();
  )
  CATCH(e,
    printf("Error caught! %s: %i\n  from function %s at %s:%i\n", e.descr, e.code, e.func, e.file, e.line);
  )
}

int main() {
  ERROR_DESCRIPTION(SOME_ERROR, "Some error");
  some_func();
  other_func();
  return 0;
}
