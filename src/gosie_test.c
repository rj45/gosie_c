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
     "binary(sub,\n"
     "  binary(add, literal(3), literal(5)),\n"
     "  literal(7)\n"
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

const testcase codegenTests[] = {
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

    tokenizerInit(&tokenizer,
                  (Source){codegenTests[i].src, strlen(codegenTests[i].src)});
    astInit(&ast, (Source){codegenTests[i].src, strlen(codegenTests[i].src)});
    irInit(&ir, &ast);
    irBuilderInit(&builder, &ir);

    parse(&tokenizer, &ast);
    irBuilderBuild(&builder);

    char buffer[1024];
    char *end = buffer + sizeof(buffer) - 1;
    genCode(buffer, end, &ir);

    ASSERT_STREQ_MSG(codegenTests[i].result, buffer, codegenTests[i].name);

    irFree(&ir);
    astFree(&ast);
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
