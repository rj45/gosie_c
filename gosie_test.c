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

UTEST(AST, creation) {
  // char buffer[1024];
  Tokenizer tokenizer;
  AST ast;
  tokenizerInit(&tokenizer, "42");
  astInit(&ast, tokenizer);

  Token token = nextToken(&tokenizer);

  astAddNode(&ast, LITERAL, token);

  ASSERT_EQ(ast.numNodes, 1);

  uint32_t nodetype = ast.nodes[0].type;
  ASSERT_EQ(nodetype, LITERAL);

  // astDump(&ast, astRootNode(&ast), buffer, sizeof(buffer), NULL);

  // ASSERT_STREQ(buffer, "literal(42)");

  astFree(&ast);
}