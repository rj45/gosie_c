#include "gosie.h"
#include "utest.h"

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
