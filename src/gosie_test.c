#include "gosie.h"
#include "utest.h"
#include <string.h>

typedef struct TestCase {
  const char *name;
  const char *src;
  const char *result;
} TestCase;

const TestCase tests[] = {
    {"int literal 42", "42", "literal(42)"},
    {"add 2 and 5", "2+5", "binary(add, literal(2), literal(5))"},
    {"add 3 and 5 and subtract 7", "3+5-7",
     "binary(sub,\n"
     "  binary(add, literal(3), literal(5)),\n"
     "  literal(7)\n"
     ")"},
};

UTEST(Parser, testCases) {
  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
    Tokenizer tokenizer;
    AST ast;
    ErrorList errs;
    Source src = (Source){tests[i].src, strlen(tests[i].src)};

    errInit(&errs, src);
    tokenizerInit(&tokenizer, src, &errs);
    astInit(&ast, src);

    parse(&tokenizer, &ast);

    ASSERT_FALSE(errHasErrors(&errs));

    char buffer[1024];
    char *end = buffer + sizeof(buffer) - 1;
    astDump(&ast, astRootNode(&ast), 0, buffer, end);

    astFree(&ast);
    errFree(&errs);

    ASSERT_STREQ_MSG(tests[i].result, buffer, tests[i].name);
  }
}

const TestCase irTests[] = {
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
     "v2 = add v0, v1\n"
     "v3 = int 7\n"
     "v4 = sub v2, v3\n"
     "v5 = error v4\n"},
};

UTEST(IR, irTests) {
  for (size_t i = 0; i < sizeof(irTests) / sizeof(irTests[0]); i++) {
    Tokenizer tokenizer;
    AST ast;
    IR ir;
    IRBuilder builder;
    ErrorList errs;
    Source src = (Source){irTests[i].src, strlen(irTests[i].src)};

    errInit(&errs, src);
    tokenizerInit(&tokenizer, src, &errs);
    astInit(&ast, src);
    irInit(&ir, &ast);
    irBuilderInit(&builder, &ir);

    parse(&tokenizer, &ast);
    ASSERT_FALSE(errHasErrors(&errs));

    irBuilderBuild(&builder);

    char buffer[1024];
    char *end = buffer + sizeof(buffer) - 1;
    irDump(&ir, buffer, end);

    errFree(&errs);
    irFree(&ir);
    astFree(&ast);

    ASSERT_STREQ_MSG(irTests[i].result, buffer, irTests[i].name);
  }
}

const TestCase codegenTests[] = {
    {"int literal 42", "42",
     "move a0, 42\n"
     "error\n"},
    {"add 2 and 5", "2+5",
     "move a0, 2\n"
     "add a0, 5\n"
     "error\n"},
    {"add 3 and 5 and subtract 7", "3+5-7",
     "move a0, 3\n"
     "add a0, 5\n"
     "sub a0, 7\n"
     "error\n"},
};

UTEST(genCode, codeGeneration) {
  for (size_t i = 0; i < sizeof(codegenTests) / sizeof(codegenTests[0]); i++) {
    Tokenizer tokenizer;
    AST ast;
    IR ir;
    IRBuilder builder;
    ErrorList errs;
    Source src = (Source){codegenTests[i].src, strlen(codegenTests[i].src)};

    errInit(&errs, src);
    tokenizerInit(&tokenizer, src, &errs);
    astInit(&ast, src);
    irInit(&ir, &ast);
    irBuilderInit(&builder, &ir);

    parse(&tokenizer, &ast);

    ASSERT_FALSE(errHasErrors(&errs));

    irBuilderBuild(&builder);

    char buffer[1024];
    char *end = buffer + sizeof(buffer) - 1;
    genCode(buffer, end, &ir);

    ASSERT_FALSE(errHasErrors(&errs));

    errFree(&errs);
    irFree(&ir);
    astFree(&ast);

    ASSERT_STREQ_MSG(codegenTests[i].result, buffer, codegenTests[i].name);
  }
}

typedef struct endToEndTestcase {
  const char *name;
  const char *src;
  int result;
} endToEndTestcase;

const endToEndTestcase endToEndTests[] = {
    {"int literal 0", "0", 0},
    {"int literal 42", "42", 42},
    {"expression 5+20-4", "5+20-4", 21},
    {"expression 12-14+5", "12-14+5", 3},
    {"expression with spaces", " 12 + 34 - 5 ", 41},
};

UTEST(compileAndRun, endToEnd) {
  for (size_t i = 0; i < sizeof(endToEndTests) / sizeof(endToEndTests[0]);
       i++) {
    int result = compileAndRun(endToEndTests[i].src);
    ASSERT_EQ_MSG(endToEndTests[i].result, result, endToEndTests[i].name);
  }
}

TestCase errorTests[] = {
    {"unexpected character", "\e", "unexpected character '\e'"},
    {"expected primary", "\e", "expected primary"},
    {"expected eof", "1 1", "expected eof token, got int"},
};

UTEST(Parser, errorCases) {
  for (size_t i = 0; i < sizeof(errorTests) / sizeof(errorTests[0]); i++) {
    Tokenizer tokenizer;
    AST ast;
    Source src = (Source){errorTests[i].src, strlen(errorTests[i].src)};
    ErrorList errs;

    errInit(&errs, src);
    tokenizerInit(&tokenizer, src, &errs);
    astInit(&ast, src);

    parse(&tokenizer, &ast);

    ASSERT_TRUE_MSG(errHasErrors(&errs), "expected to have errors");

    char buffer[1024];
    char *start = buffer;
    char *end = buffer + sizeof(buffer) - 1;

    for (size_t i = 0; i < arrlen(errs.errors); i++) {
      Error error = errs.errors[i];
      start = seprintf(start, end, "%d: %s\n", error.token.position, error.msg);
    }

    char *result = strnstr(buffer, errorTests[i].result, buffer - start);
    if (result == NULL) {
      fprintf(stderr, "expected to contain: %s\n", errorTests[i].result);
      fprintf(stderr, "got: %s\n", buffer);
    }
    ASSERT_NE_MSG(NULL, result, errorTests[i].name);

    astFree(&ast);
    errFree(&errs);
  }
}