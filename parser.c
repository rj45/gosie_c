#include "gosie.h"

#include <stdlib.h>

typedef struct Parser {
  Tokenizer *tokenizer;
  AST *ast;
} Parser;

static TokenType peekTokenType(Parser *parser) {
  return tokenPeek(parser->tokenizer).type;
}

static Token expectToken(Parser *parser, TokenType type) {
  Token token = tokenNext(parser->tokenizer);
  if (token.type != type) {
    fprintf(stderr, "error: expected %s token, got %s\n", tokenTypeString(type),
            tokenTypeString(token.type));
    exit(1);
  }
  return token;
}

static NodeID parsePrimary(Parser *parser) {
  switch (peekTokenType(parser)) {
  case TK_INT:
    return astAddNode(parser->ast, LITERAL, expectToken(parser, TK_INT));
  default:
    fprintf(stderr, "error: expected primary, got %s\n",
            tokenTypeString(peekTokenType(parser)));
    exit(1);
  }
}

static NodeID parseBinary(Parser *parser) {
  NodeRes res = astReserveNode(parser->ast);
  NodeCtx ctx;

  NodeID left = parsePrimary(parser);

  for (;;) {
    switch (peekTokenType(parser)) {
    case TK_ADD:
    case TK_SUB:
      ctx =
          astInsertNode(parser->ast, res, BINARY, tokenNext(parser->tokenizer));

      break;
    default:
      return left;
    }

    parsePrimary(parser);
    left = astEndNode(parser->ast, ctx);
  }

  return left;
}

void parse(Tokenizer *tokenizer, AST *ast) {
  Parser parser = {.tokenizer = tokenizer, .ast = ast};

  parseBinary(&parser);

  expectToken(&parser, TK_EOF);
}