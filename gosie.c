#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

void initTokenizer(Tokenizer *tokenizer, const char *src) {
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

Token findNextToken(Tokenizer *tokenizer, Token token) {
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

const char *tokenString(Tokenizer tokenizer, Token token) {
  return tokenizer.src + token.position;
}

int main(int argc, const char *argv[]) {
  assert(sizeof(Token) == 4);

  Tokenizer tokenizer;
  initTokenizer(&tokenizer, argv[1]);

  for (;;) {
    Token token = nextToken(&tokenizer);
    switch (token.type) {
    case TK_WHITESPACE: // fallthrough
    case TK_INVALID:
      fprintf(stderr, "error: invalid token\n");
      return 1;
    case TK_INT:
      printf("move a0, %d\n", atoi(tokenString(tokenizer, token)));
      break;
    case TK_ADD:
      token = nextToken(&tokenizer);
      printf("add a0, %d\n", atoi(tokenString(tokenizer, token)));
      break;
    case TK_SUB:
      token = nextToken(&tokenizer);
      printf("sub a0, %d\n", atoi(tokenString(tokenizer, token)));
      break;
    case TK_EOF:
      printf("error\n");
      return 0;
    }
  }
}
