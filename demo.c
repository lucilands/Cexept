#define CEXEPT_IMPLEMENTATION
#include "cexept.h"


#define SOME_ERROR 69

void other_func() {
  printf("before error\n");
  CEXEPT_THROW(SOME_ERROR);
  printf("after raise\n");
}

void some_func() {
  CEXEPT_TRY(
    other_func();
  )
  CEXEPT_CATCH(e,
    printf("Error caught! %s: %i\n", e.descr, e.code);
  )
}

int main() {
  some_func();
  other_func();
  return 0;
}
