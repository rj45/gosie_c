#include "gosie.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenizerInit(Tokenizer *tokenizer, const char *src) {
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

int findTokenEnd(const Tokenizer *tokenizer, Token token) {
  int index = token.position;
  const char *src = tokenizer->src + index;
  while (tokenType(*src) == token.type) {
    src++;
    index++;
  }
  return index;
}

Token findNextToken(const Tokenizer *tokenizer, Token token) {
  int index = findTokenEnd(tokenizer, token);
  return (Token){.type = tokenType(tokenizer->src[index]), .position = index};
};

Token nextToken(Tokenizer *tokenizer) {
  Token token = tokenizer->token;
  do {
    token = findNextToken(tokenizer, token);
  } while (token.type == TK_WHITESPACE);
  tokenizer->token = token;
  return token;
}

const char *tokenStringNoNull(Tokenizer *tokenizer, Token token) {
  return tokenizer->src + token.position;
}

char *tokenString(Tokenizer *tokenizer, Token token, char *buffer, char *end) {
  int len = findTokenEnd(tokenizer, token) - token.position;
  return seprintf(buffer, end, "%.*s", len,
                  tokenStringNoNull(tokenizer, token));
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