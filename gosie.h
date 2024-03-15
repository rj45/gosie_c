#ifndef GOSIE_H
#define GOSIE_H

#include <stdint.h>

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

typedef struct Tokenizer {
  const char *src;
  Token token;
} Tokenizer;

void tokenizerInit(Tokenizer *tokenizer, const char *src);
int findTokenEnd(const Tokenizer *tokenizer, Token token);
Token findNextToken(const Tokenizer *tokenizer, Token token);
Token nextToken(Tokenizer *tokenizer);
const char *tokenStringNoNull(Tokenizer *tokenizer, Token token);
const char *tokenString(Tokenizer *tokenizer, Token token, char *buffer,
                        int maxSize, int *size);

#pragma endregion

#pragma region AST

typedef uint32_t NodeID;

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

  Tokenizer tokenizer;
} AST;

typedef struct NodeCtx {
  NodeID node;
  int numNodes;
} NodeCtx;

void astInit(AST *ast, Tokenizer tokenizer);
void astFree(AST *ast);

NodeID astRootNode(AST *ast);
NodeCtx astStartNode(AST *ast, NodeType type, Token token);
void astEndNode(AST *ast, NodeCtx ctx);
void astAddNode(AST *ast, NodeType type, Token token);

void astDump(AST *ast, NodeID node, char *buffer, int maxSize, int *size);

#pragma endregion

#endif