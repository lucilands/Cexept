#define CEXEPT_IMPLEMENTATION
#include "cexept.h"


#define SOME_ERROR 69

void other_func() {
  CEXEPT_RAISE(SOME_ERROR);
}

void some_func() {
  CEXEPT_TRY(
    other_func();
  )
  cexception_t e;
  CEXEPT_CATCH(e,
    printf("Error caught! %s: %i\n", e.descr, e.code);
  )
}

int main() {
  some_func();
  CEXEPT_RAISE(10);
  return 0;
}
