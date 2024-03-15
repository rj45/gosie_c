#include "gosie.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[]) {
  assert(sizeof(Token) == 4);

  Tokenizer tokenizer;
  tokenizerInit(&tokenizer, argv[1]);

  for (;;) {
    Token token = nextToken(&tokenizer);
    switch (token.type) {
    case TK_WHITESPACE: // fallthrough
    case TK_INVALID:
      fprintf(stderr, "error: invalid token\n");
      return 1;
    case TK_INT:
      printf("move a0, %d\n", atoi(tokenStringNoNull(&tokenizer, token)));
      break;
    case TK_ADD:
      token = nextToken(&tokenizer);
      printf("add a0, %d\n", atoi(tokenStringNoNull(&tokenizer, token)));
      break;
    case TK_SUB:
      token = nextToken(&tokenizer);
      printf("sub a0, %d\n", atoi(tokenStringNoNull(&tokenizer, token)));
      break;
    case TK_EOF:
      printf("error\n");
      return 0;
    }
  }
}
