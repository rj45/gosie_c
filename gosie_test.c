#include "gosie.h"
#include "utest.h"

#include <assert.h>

typedef struct testcase {
  const char *name;
  const char *src;
  const char *result;
} testcase;

const testcase tests[] = {
    {"int literal 42", "42", "literal(42)"},
    {"add 2 and 5", "2+5", "binary(add, literal(2), literal(5))"},
    {"add 3 and 5 and subtract 7", "3+5-7",
     "binary(add,\n"
     "  literal(3),\n"
     "  binary(sub, literal(5), literal(7))\n"
     ")"},
};

UTEST(Parser, testCases) {
  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
    Tokenizer tokenizer;
    AST ast;
    tokenizerInit(&tokenizer, (Source){tests[i].src, strlen(tests[i].src)});
    astInit(&ast, (Source){tests[i].src, strlen(tests[i].src)});

    parse(&tokenizer, &ast);

    char buffer[1024];
    char *end = buffer + sizeof(buffer) - 1;
    astDump(&ast, astRootNode(&ast), 0, buffer, end);

    ASSERT_STREQ_MSG(tests[i].result, buffer, tests[i].name);

    astFree(&ast);
  }
}

const testcase irTests[] = {
    {"int literal 42", "42",
     "v0 = int 42\n"
     "v1 = error v0\n"},
    {"add 2 and 5", "2+5",
     "v0 = int 2\n"
     "v1 = int 5\n"
     "v2 = add v0, v1\n"
     "v3 = error v2\n"},
    {"add 3 and 5 and subtract 7", "3+5-7",
     "v0 = int 3\n"
     "v1 = int 5\n"
     "v2 = int 7\n"
     "v3 = sub v1, v2\n"
     "v4 = add v0, v3\n"
     "v5 = error v4\n"},
};

UTEST(IR, irTests) {
  for (size_t i = 0; i < sizeof(irTests) / sizeof(irTests[0]); i++) {
    Tokenizer tokenizer;
    AST ast;
    IR ir;
    IRBuilder builder;

    tokenizerInit(&tokenizer, (Source){irTests[i].src, strlen(irTests[i].src)});
    astInit(&ast, (Source){irTests[i].src, strlen(irTests[i].src)});
    irInit(&ir, &ast);
    irBuilderInit(&builder, &ir);

    parse(&tokenizer, &ast);
    irBuilderBuild(&builder);

    char buffer[1024];
    char *end = buffer + sizeof(buffer) - 1;
    irDump(&ir, buffer, end);

    ASSERT_STREQ_MSG(irTests[i].result, buffer, irTests[i].name);

    irFree(&ir);
    astFree(&ast);
  }
}