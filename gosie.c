#include "gosie.h"

#include <stdio.h>

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <source>\n", argv[0]);
    return 1;
  }

  return compileAndRun(argv[1]);
}
