#include "gosie.h"

void tokenizerInit(Tokenizer *tokenizer, Source src) {
  tokenizer->src = src;
  tokenizer->token = (Token){.type = TK_INVALID, .position = 0};
}

static TokenType tokenType(char c) {
  switch (c) {
  case '\0':
    return TK_EOF;
  case ' ':  // fallthrough
  case '\t': // fallthrough
  case '\n': // fallthrough
  case '\r':
    return TK_WHITESPACE;
  case '0' ... '9':
    return TK_INT;
  case '+':
    return TK_ADD;
  case '-':
    return TK_SUB;
  default:
    return TK_INVALID;
  }
}

Token tokenFindNext(const Tokenizer *tokenizer, Token token) {
  int index = srcFindTokenEnd(tokenizer->src, token);
  return (Token){.type = tokenType(tokenizer->src.src[index]),
                 .position = index};
};

Token tokenPeek(Tokenizer *tokenizer) {
  Token token = tokenizer->token;
  do {
    token = tokenFindNext(tokenizer, token);
  } while (token.type == TK_WHITESPACE);
  return token;
}

Token tokenNext(Tokenizer *tokenizer) {
  Token token = tokenPeek(tokenizer);
  tokenizer->token = token;
  return token;
}

int srcFindTokenEnd(Source source, Token token) {
  int index = token.position;
  const char *src = source.src + index;
  while (tokenType(*src) == token.type) {
    src++;
    index++;
  }
  return index;
}

const char *srcTokenStringNoNull(Source src, Token token) {
  if (token.position >= src.len) {
    return "";
  }
  return src.src + token.position;
}

char *srcTokenString(char *start, char *end, Source src, Token token) {
  int len = srcFindTokenEnd(src, token) - token.position;
  return seprintf(start, end, "%.*s", len, srcTokenStringNoNull(src, token));
}

static const char *typeStrings[] = {
    [TK_WHITESPACE] = "whitespace",
    [TK_INVALID] = "invalid",
    [TK_INT] = "int",
    [TK_ADD] = "add",
    [TK_SUB] = "sub",
    [TK_EOF] = "eof",
};

const char *tokenTypeString(TokenType type) { return typeStrings[type]; }