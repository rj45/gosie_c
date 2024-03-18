#include "gosie.h"
#include "utest.h"

// typedef struct testcase {
//   const char *name;
//   const char *src;
//   const char *result;
// } testcase;

// const testcase tests[] = {
//     {"int literal 42", "42", "literal(42)"},
//     {"add 2 and 5", "2+5", "binary(add, literal(2), literal(5))"},
// };

UTEST(AST, liternal) {
  char buffer[1024];
  Tokenizer tokenizer;
  AST ast;
  tokenizerInit(&tokenizer, "42");
  astInit(&ast, tokenizer);

  Token token = nextToken(&tokenizer);

  astAddNode(&ast, LITERAL, token);

  ASSERT_EQ(ast.numNodes, 1);

  uint32_t nodetype = ast.nodes[0].type;
  ASSERT_EQ(nodetype, LITERAL);

  char *end = buffer + sizeof(buffer) - 1;

  astDump(&ast, astRootNode(&ast), 0, buffer, end);

  ASSERT_STREQ(buffer, "literal(42)");

  astFree(&ast);
}

UTEST(AST, binary) {
  char buffer[1024];
  Tokenizer tokenizer;
  AST ast;
  tokenizerInit(&tokenizer, "3+5");
  astInit(&ast, tokenizer);

  Token token = nextToken(&tokenizer);

  NodeCtx ctx = astStartNode(&ast, BINARY, nextToken(&tokenizer));

  astAddNode(&ast, LITERAL, token);

  token = nextToken(&tokenizer);
  astAddNode(&ast, LITERAL, token);

  astEndNode(&ast, ctx);

  ASSERT_EQ(ast.numNodes, 3);

  char *end = buffer + sizeof(buffer) - 1;

  astDump(&ast, astRootNode(&ast), 0, buffer, end);

  ASSERT_STREQ("binary(add, literal(3), literal(5))", buffer);

  astFree(&ast);
}

UTEST(AST, binaryWithIndent) {
  char buffer[1024];
  Tokenizer tokenizer;
  AST ast;
  tokenizerInit(&tokenizer, "3+5-7");
  astInit(&ast, tokenizer);

  Token token = nextToken(&tokenizer);

  NodeCtx ctx1 = astStartNode(&ast, BINARY, nextToken(&tokenizer));

  astAddNode(&ast, LITERAL, token);

  token = nextToken(&tokenizer);

  NodeCtx ctx2 = astStartNode(&ast, BINARY, nextToken(&tokenizer));

  astAddNode(&ast, LITERAL, token);

  token = nextToken(&tokenizer);
  astAddNode(&ast, LITERAL, token);

  astEndNode(&ast, ctx2);

  astEndNode(&ast, ctx1);

  ASSERT_EQ(ast.numNodes, 5);

  char *end = buffer + sizeof(buffer) - 1;

  astDump(&ast, astRootNode(&ast), 0, buffer, end);

  ASSERT_STREQ("binary(add,\n"
               "  literal(3),\n"
               "  binary(sub, literal(5), literal(7))\n"
               ")",
               buffer);

  astFree(&ast);
}