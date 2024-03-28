#include "gosie.h"

#include <stdbool.h>

void astInit(AST *ast, Source src) { *ast = (AST){.src = src}; }

void astFree(AST *ast) { arrfree(ast->nodes); }

NodeID astRootNode(const AST *ast) {
  if (arrlen(ast->nodes) > 0) {
    return 0;
  }
  return NO_NODE;
}

NodeRes astReserveNode(AST *ast) {
  return (NodeRes){.node = arrlen(ast->nodes)};
}

NodeCtx astStartNode(AST *ast, NodeType type, Token token) {
  NodeID id = arrlen(ast->nodes);
  Node node = (Node){.type = type, .token = token};
  arrput(ast->nodes, node);

  return (NodeCtx){.node = id};
}

NodeCtx astInsertNode(AST *ast, NodeRes reserve, NodeType type, Token token) {
  Node node = (Node){.type = type, .token = token};
  arrins(ast->nodes, reserve.node, node);

  return (NodeCtx){.node = reserve.node};
}

NodeID astEndNode(AST *ast, NodeCtx ctx) {
  uint32_t subNodes = arrlen(ast->nodes) - (ctx.node + 1);
  ast->nodes[ctx.node].subNodes = subNodes;
  return ctx.node;
}

NodeID astAddNode(AST *ast, NodeType type, Token token) {
  NodeCtx ctx = astStartNode(ast, type, token);
  return astEndNode(ast, ctx);
}

Token astSetToken(AST *ast, NodeCtx ctx, Token token) {
  ast->nodes[ctx.node].token = token;
  return token;
}

const char *nodeTypeString[] = {
    [BINARY] = "binary",
    [LITERAL] = "literal",
};

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
    cur = srcTokenString(cur, end, ast->src, curNode.token);
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
