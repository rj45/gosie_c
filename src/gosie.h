#ifndef GOSIE_H
#define GOSIE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "stb_ds.h"

typedef struct ErrorList ErrorList;

#pragma region Tokenizer

typedef enum TokenType {
  TK_INVALID,
  TK_ERROR,
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
  ErrorList *errs;
} Tokenizer;

void tokenizerInit(Tokenizer *tokenizer, Source src, ErrorList *errs);
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

NodeID astRootNode(const AST *ast);
NodeCtx astStartNode(AST *ast, NodeType type, Token token);
NodeID astEndNode(AST *ast, NodeCtx ctx);
NodeID astAddNode(AST *ast, NodeType type, Token token);
NodeRes astReserveNode(AST *ast);
NodeCtx astInsertNode(AST *ast, NodeRes reserve, NodeType type, Token token);
Token astSetToken(AST *ast, NodeCtx ctx, Token token);

typedef struct ChildIter {
  AST *ast;
  uint32_t subnodesLeft;
  NodeID node;
} ChildIter;
ChildIter astNewChildIter(AST *ast, NodeID node);
NodeID astCurChild(ChildIter iter);
ChildIter astNextChild(ChildIter iter);

char *astDump(AST *ast, NodeID node, int indent, char *start, char *end);

#pragma endregion

#pragma region Parser

void parse(Tokenizer *tokenizer, AST *ast);

#pragma endregion

#pragma region IR

typedef enum Op {
  OP_INVALID,
  OP_INT,
  OP_ADD,
  OP_SUB,
  OP_ERROR,
} Op;

typedef enum InputType {
  INPUT_NONE,
  INPUT_ONE,
  INPUT_TWO,
  INPUT_INT,
} InputType;

typedef struct OpDef {
  const char *name;
  InputType inputType;
} OpDef;

const OpDef *opDef(Op op);

typedef uint32_t InstrID;
static const InstrID NO_INSTR = 0xffffffff;

typedef struct Instr {
  Op op;
  NodeID astNode;
  union {
    uint64_t intConst;
    InstrID inputs[2];
  };
} Instr;

typedef struct IR {
  Instr *instrs; // stb dynamic array
  AST *ast;
} IR;

void irInit(IR *ir, AST *ast);
void irFree(IR *ir);
InstrID irAddInstr(IR *ir, Op op, NodeID astNode);
InstrID irSetInt(IR *ir, InstrID instr, uint64_t value);
InstrID irSetInput1(IR *ir, InstrID instr, InstrID input);
InstrID irSetInput2(IR *ir, InstrID instr, InstrID input1, InstrID input2);
char *irPrintInstr(char *start, char *end, IR *ir, InstrID instr);
char *irDump(IR *ir, char *start, char *end);

typedef struct IRBuilder {
  AST *ast;
  IR *ir;
} IRBuilder;

void irBuilderInit(IRBuilder *builder, IR *ir);
void irBuilderFree(IRBuilder *builder);
void irBuilderBuild(IRBuilder *builder);

void irBuilderBuild(IRBuilder *builder);

char *genCode(char *start, char *end, IR *ir);

#pragma endregion

#pragma region Compile

int compileAndRun(const char *source);

#pragma endregion

#pragma region Err

typedef struct Error {
  const char *msg;
  Token token;
} Error;

struct ErrorList {
  Error *errors;
  Source src;
};

void errInit(ErrorList *err, Source src);
void errFree(ErrorList *err);
bool errHasErrors(ErrorList *err);
void errErrorf(ErrorList *err, Token token, const char *msg, ...)
    __attribute__((format(printf, 3, 4)));
void errPrintAll(ErrorList *err);

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