#include "ccani.h"

//-----------------------------------------------------------------------------------------
//----------------------------------- tokenizer ------------------------------------
//-----------------------------------------------------------------------------------------

// 現在着目しているトークン
Token *token;


void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// curの後ろに繋がる新しいトークンの作成
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークナイズ
void tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if(strchr("+-*/()><=;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, -1);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  token = head.next;
}


//-----------------------------------------------------------------------------------------
//----------------------------------- parser ----------------------------------
//-----------------------------------------------------------------------------------------

// program    = stmt*
// stmt       = expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | ident | "(" expr ")"
// 

// 次のトークンが期待する記号opのときには読み進めて真を返す
bool consume_reserved(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待する記号opのときには読み進める
void expect_reserved(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected %s", op);
  token = token->next;
}

// 次のトークンが識別子のときには読み進めてオフセットを返す
int consume_ident() {
  if (token->kind != TK_IDENT)
    return 0;
  int offset = (token->str[0] - 'a' + 1) * 8;
  token = token->next;
  return offset;
}


// 次のトークンが数値の場合、読み進めて数値を返す
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(int offset) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->offset = offset;
  return node;
}

Node *code[100];

void program() {
  int i = 0;
  while (!at_eof()){
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node *stmt() {
  Node *node = expr();
  expect_reserved(";");
  return node;
}

Node *expr() {
  Node *node = assign();
}

Node *assign() {
  Node *node = equality();
  if (consume_reserved("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume_reserved("=="))
      node = new_node(ND_EQ, node, add());
    else if (consume_reserved("!="))
      node = new_node(ND_NE, node, add());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume_reserved("<"))
      node = new_node(ND_LT, node, add());
    else if (consume_reserved("<="))
      node = new_node(ND_LE, node, add());
    else if (consume_reserved(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume_reserved(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume_reserved("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume_reserved("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume_reserved("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume_reserved("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume_reserved("+"))
    return primary();
  else if (consume_reserved("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  else
    return primary();

}

Node *primary() {
  if (consume_reserved("(")){
    Node *node = expr();
    expect_reserved(")");
    return node;
  }
  int offset = consume_ident();
  if (offset > 0)
    return new_node_ident(offset);

  int val = expect_number();
  return new_node_num(val);
}
