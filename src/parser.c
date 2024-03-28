#include "gosie.h"

typedef struct Parser {
  Tokenizer *tokenizer;
  AST *ast;
  ErrorList *errs;
} Parser;

static TokenType peekTokenType(Parser *parser) {
  return tokenPeek(parser->tokenizer).type;
}

static Token expectToken(Parser *parser, TokenType type) {
  Token token = tokenNext(parser->tokenizer);
  if (token.type != type) {
    errErrorf(parser->errs, token, "expected %s token, got %s\n",
              tokenTypeString(type), tokenTypeString(token.type));
  }
  return token;
}

static NodeID parsePrimary(Parser *parser) {
  switch (peekTokenType(parser)) {
  case TK_INT:
    return astAddNode(parser->ast, LITERAL, expectToken(parser, TK_INT));
  default: {
    Token errtok = tokenPeek(parser->tokenizer);
    errErrorf(parser->errs, errtok, "expected primary, got %s\n",
              tokenTypeString(errtok.type));
    return NO_NODE;
  }
  }
}

int precedence[] = {
    [TK_ADD] = 4, [TK_SUB] = 4, [TK_AMP] = 8, [TK_OR] = 9, [TK_XOR] = 10,
};

static NodeID parseBinary(Parser *parser, int minPrecedence) {
  NodeRes res = astReserveNode(parser->ast);
  NodeCtx ctx;

  NodeID left = parsePrimary(parser);

  for (;;) {
    TokenType type = peekTokenType(parser);
    switch (type) {
    case TK_ADD:
    case TK_SUB:
    case TK_AMP:
    case TK_OR:
    case TK_XOR:
      if (precedence[type] <= minPrecedence) {
        return left;
      }
      ctx =
          astInsertNode(parser->ast, res, BINARY, tokenNext(parser->tokenizer));

      break;
    default:
      return left;
    }

    parseBinary(parser, precedence[type]);
    left = astEndNode(parser->ast, ctx);
  }

  return left;
}

void parse(Tokenizer *tokenizer, AST *ast) {
  Parser parser = {.tokenizer = tokenizer, .ast = ast, .errs = tokenizer->errs};

  parseBinary(&parser, 0);

  expectToken(&parser, TK_EOF);
}