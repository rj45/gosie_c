#include "gosie.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void astInit(AST *ast, Tokenizer tokenizer) {
  memset(ast, 0, sizeof(AST));
  ast->tokenizer = tokenizer;
}

void astFree(AST *ast) { free(ast->nodes); }

NodeID astRootNode(AST *ast) {
  if (ast->numNodes > 0) {
    return 0;
  }
  return NO_NODE;
}

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

const char *nodeTypeString[] = {
    [BINARY] = "binary",
    [LITERAL] = "literal",
};

typedef struct ChildIter {
  AST *ast;
  uint32_t subnodesLeft;
  NodeID node;
} ChildIter;

ChildIter astNewChildIter(AST *ast, NodeID node) {
  return (ChildIter){
      .ast = ast, .subnodesLeft = ast->nodes[node].subNodes, .node = node + 1};
}

NodeID astCurChild(ChildIter iter) {
  if (iter.subnodesLeft == 0) {
    return NO_NODE;
  }
  return iter.node;
}

ChildIter astNextChild(ChildIter iter) {
  if (iter.subnodesLeft == 0) {
    return iter;
  }
  uint32_t subnodes = iter.ast->nodes[iter.node].subNodes + 1;
  iter.subnodesLeft -= subnodes;
  iter.node += subnodes;
  return iter;
}

bool astHasNextChild(ChildIter iter) {
  return iter.subnodesLeft - iter.ast->nodes[iter.node].subNodes > 1;
}

char *printIndent(char *cur, char *end, int indent) {
  for (int i = 0; i < indent; i++) {
    cur = seprintf(cur, end, "  ");
  }
  return cur;
}

char *astDump(AST *ast, NodeID node, int indent, char *start, char *end) {
  Node curNode = ast->nodes[node];
  char *cur = start;

  cur = printIndent(cur, end, indent);

  cur = seprintf(cur, end, "%s(", nodeTypeString[curNode.type]);

  int nextIndent = indent + 1;
  if (curNode.subNodes < 3) {
    nextIndent = 0;
  }

  bool skipComma = true;
  if (curNode.type == LITERAL) {
    cur = tokenString(&ast->tokenizer, curNode.token, cur, end);
  } else if (curNode.type == BINARY) {
    cur = seprintf(cur, end, "%s,", tokenTypeString(curNode.token.type));
    if (nextIndent == 0) {
      cur = seprintf(cur, end, " ");
    }
    skipComma = false;
  }

  if (nextIndent > 0) {
    cur = seprintf(cur, end, "\n");
  }

  ChildIter iter = astNewChildIter(ast, node);
  for (NodeID child = astCurChild(iter); child != NO_NODE;
       iter = astNextChild(iter), child = astCurChild(iter)) {

    cur = astDump(ast, child, nextIndent, cur, end);
    if (!skipComma && astHasNextChild(iter)) {
      cur = seprintf(cur, end, ",");
    }
    skipComma = false;
    if (nextIndent > 0) {
      cur = seprintf(cur, end, "\n");
    } else if (astHasNextChild(iter)) {
      cur = seprintf(cur, end, " ");
    }
  }

  if (nextIndent > 0) {
    cur = printIndent(cur, end, indent);
  }
  cur = seprintf(cur, end, ")");
  return cur;
}
