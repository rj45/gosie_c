#include "gosie.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void astInit(AST *ast, Tokenizer tokenizer) {
  memset(ast, 0, sizeof(AST));
  ast->tokenizer = tokenizer;
}

void astFree(AST *ast) { free(ast->nodes); }

NodeCtx astStartNode(AST *ast, NodeType type, Token token) {
  if (ast->numNodes == ast->capacity) {
    ast->capacity *= 2;
    ast->nodes = realloc(ast->nodes, ast->capacity * sizeof(Node));
    ast->types = realloc(ast->types, ast->capacity * sizeof(ValueType));
  }

  NodeID node = ast->numNodes;
  ast->numNodes++;
  ast->nodes[node].type = type;
  ast->nodes[node].token = token;

  return (NodeCtx){.node = node, .numNodes = ast->numNodes};
}

void astEndNode(AST *ast, NodeCtx ctx) {
  ast->nodes[ctx.node].subNodes = ast->numNodes - ctx.numNodes;
}

void astAddNode(AST *ast, NodeType type, Token token) {
  NodeCtx ctx = astStartNode(ast, type, token);
  astEndNode(ast, ctx);
}

void astDump(AST *ast, NodeID node, char *buffer, int maxSize, int *size) {
  assert(maxSize > 0);
}
