#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

typedef enum {
  TK_IDENT,    // 識別子
  TK_PUNCT,    // 区切り記号
  TK_KEYWORD,  // キーワード
  TK_NUM,      // 整数トークン
  TK_EOF,      // 終端記号
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;  // 型
  Token *next;     // 次の入力トークン
  int val;         // kindがTK_NUMの場合の値
  char *str;       // トークン文字列
  int len;         // トークンの長さ
};

void error_at(char *loc, char *fmt, ...);
void tokenize();

extern Token *token;

//
// parser.c
//

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,      // +
  ND_SUB,      // -
  ND_MUL,      // *
  ND_DIV,      // /
  ND_RMD,      // %
  ND_EQ,       // ==
  ND_NE,       // !=
  ND_LT,       // <
  ND_LE,       // <=
  ND_ASSIGN,   // =
  ND_LVAR,     // Local Variable
  ND_NUM,      // Integer
  ND_RETURN,   // return Statement
  ND_IF,       // if Statement
  ND_FOR,      // for/while Statement
  ND_BLOCK,    // Block (Conpund Statement)
  ND_FUNCALL,  // Function Call
  ND_FUNDEF,   // Function Definition

} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;     // NUM
  int offset;  // LVAR

  // IF or FOR
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  // BLOCK (make a vector)
  Node *next;

  // FUNCALL (make a vector)
  Node *next_arg;

  // FUNCALL, FUNDEF
  char *fname;

  // FUNDEF
  int args_offset[6];
  Node *code[100];
};

void program();
Node *func();
Node *stmt();
Node *expr();
Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

extern Node *funcs[100];
//
// codegen.c
//

void gen(Node *node);

//
// main.c
//

extern char *user_input;
