#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 終端記号
} TokenKind;

typedef struct Token Token;

struct Token{
  TokenKind kind; // 型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合の値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ, // ==
  ND_NE, // !=
  ND_LT, // < 
  ND_LE, // <=
  ND_NUM, // Integer
} NodeKind;

typedef struct Node Node;

struct Node{
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

extern char *user_input;
extern Token *token;

Token *tokenize();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);