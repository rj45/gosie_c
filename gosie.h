#ifndef GOSIE_H
#define GOSIE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#pragma region Tokenizer

typedef enum TokenType {
  TK_INVALID,
  TK_EOF,
  TK_WHITESPACE,
  // TK_COMMENT,
  TK_INT,
  TK_ADD,
  TK_SUB,
} TokenType;

typedef struct Token {
  uint32_t position : 24;
  TokenType type : 8;
} Token;

static const Token INVALID_TOKEN = {.type = TK_INVALID, .position = 0};

typedef struct Source {
  const char *src;
  int len;
} Source;

int srcFindTokenEnd(Source source, Token token);
const char *srcTokenStringNoNull(Source src, Token token);
char *srcTokenString(char *start, char *end, Source src, Token token);

typedef struct Tokenizer {
  Source src;
  Token token;
} Tokenizer;

void tokenizerInit(Tokenizer *tokenizer, Source src);
Token tokenFindNext(const Tokenizer *tokenizer, Token token);
Token tokenPeek(Tokenizer *tokenizer);
Token tokenNext(Tokenizer *tokenizer);
const char *tokenTypeString(TokenType type);

#pragma endregion

#pragma region AST

typedef uint32_t NodeID;
static const NodeID NO_NODE = UINT32_MAX;

typedef enum NodeType {
  BINARY,
  LITERAL,
} NodeType;

typedef struct Node {
  struct {
    NodeType type : 8;
    uint32_t subNodes : 24;
  };
  Token token;
} Node;

typedef uint32_t ValueType;

typedef struct AST {
  Node *nodes;      // array of nodes, numNodes long
  ValueType *types; // array of types, numNodes long
  int numNodes;
  int capacity;

  Source src;
} AST;

typedef struct NodeCtx {
  NodeID node;
} NodeCtx;

typedef struct NodeRes {
  NodeID node;
} NodeRes;

void astInit(AST *ast, Source src);
void astFree(AST *ast);

NodeID astRootNode(AST *ast);
NodeCtx astStartNode(AST *ast, NodeType type, Token token);
NodeID astEndNode(AST *ast, NodeCtx ctx);
NodeID astAddNode(AST *ast, NodeType type, Token token);
NodeRes astReserveNode(AST *ast);
NodeCtx astInsertNode(AST *ast, NodeRes reserve, NodeType type, Token token);
Token astSetToken(AST *ast, NodeCtx ctx, Token token);

char *astDump(AST *ast, NodeID node, int indent, char *start, char *end);

#pragma endregion

#pragma region Parser

void parse(Tokenizer *tokenizer, AST *ast);

#pragma endregion

#pragma region utils

// from: https://text.causal.agency/024-seprintf.txt
static inline char *vseprintf(char *ptr, char *end, const char *fmt,
                              va_list args) {
  int n = vsnprintf(ptr, end - ptr, fmt, args);
  if (n < 0) {
    return NULL;
  }
  if (n > end - ptr) {
    return end;
  }
  return ptr + n;
}

static inline char *seprintf(char *ptr, char *end, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));
static inline char *seprintf(char *ptr, char *end, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char *ret = vseprintf(ptr, end, fmt, ap);
  va_end(ap);
  return ret;
}

#pragma endregion

#endif