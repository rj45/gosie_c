#include "gosie.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <source>\n", argv[0]);
    return 1;
  }

  Tokenizer tokenizer;
  AST ast;
  IR ir;
  IRBuilder builder;
  Source src = (Source){argv[1], strlen(argv[1])};

  tokenizerInit(&tokenizer, src);
  astInit(&ast, src);
  irInit(&ir, &ast);
  irBuilderInit(&builder, &ir);

  parse(&tokenizer, &ast);
  irBuilderBuild(&builder);

  char buffer[1024];
  char *end = buffer + sizeof(buffer) - 1;
  genCode(buffer, end, &ir);

  printf("%s", buffer);

  irFree(&ir);
  astFree(&ast);
}
