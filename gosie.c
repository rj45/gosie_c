#include "gosie.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[]) {
  (void)argc;
  assert(sizeof(Token) == 4);

  Source src = {argv[1], strlen(argv[1])};
  Tokenizer tokenizer;
  tokenizerInit(&tokenizer, src);

  for (;;) {
    Token token = tokenNext(&tokenizer);
    switch (token.type) {
    case TK_WHITESPACE: // fallthrough
    case TK_INVALID:
      fprintf(stderr, "error: invalid token\n");
      return 1;
    case TK_INT:
      printf("move a0, %d\n", atoi(srcTokenStringNoNull(src, token)));
      break;
    case TK_ADD:
      token = tokenNext(&tokenizer);
      printf("add a0, %d\n", atoi(srcTokenStringNoNull(src, token)));
      break;
    case TK_SUB:
      token = tokenNext(&tokenizer);
      printf("sub a0, %d\n", atoi(srcTokenStringNoNull(src, token)));
      break;
    case TK_EOF:
      printf("error\n");
      return 0;
    }
  }
}
