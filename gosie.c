#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
  char *str = (char *)argv[1];
  printf("move a0, %ld\n", strtol(str, &str, 10));
  while (*str) {
    switch (*str) {
    case '-':
      printf("move a1, %ld\n", strtol(str + 1, &str, 10));
      printf("sub a0, a1\n");
      break;

    case '+':
      printf("move a1, %ld\n", strtol(str + 1, &str, 10));
      printf("add a0, a1\n");
      break;
    }
  }
  printf("error\n");
  return 0;
}
