#include "ccani.h"

// program    = stmt*
// stmt       = expr ";" | "return" expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | ident | "(" expr ")"
//

typedef struct LVar LVar;

struct LVar {
  LVar *next;  // 次の変数 or NULL
  char *name;  // 変数名;
  int len;     // 名前の長さ
  int offset;  // rbpからのオフセット
};

LVar *locals;

// 変数を名前で検索し、見つからなければNULLを返す
LVar *find_lvar(Token *tok) {
  LVar *lvar = locals;
  for (LVar *lvar = locals; lvar; lvar = lvar->next) {
    if (lvar->len == tok->len && !memcmp(tok->str, lvar->name, lvar->len))
      return lvar;
  }

  return NULL;
}

// 次のトークンが識別子のときには読み進めてオフセットを返す
int consume_ident() {
  if (token->kind != TK_IDENT) return -1;

  LVar *lvar = find_lvar(token);
  if (!lvar) {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = token->str;
    lvar->len = token->len;
    lvar->offset = locals ? (locals->offset + 8) : 0;
    locals = lvar;
  }
  token = token->next;
  return lvar->offset;
}

// 次のトークンが期待する記号opのときには読み進めて真を返す
bool consume_punct(char *op) {
  if (token->kind != TK_PUNCT || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待する記号opのときには読み進める
void expect_punct(char *op) {
  if (token->kind != TK_PUNCT || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected %s", op);
  token = token->next;
}

// 次のトークンが数値の場合、読み進めて数値を返す
int expect_number() {
  if (token->kind != TK_NUM) error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

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
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node *stmt() {
  Node *node;

  // 本当はconsume_return(), new_node_returnを作りたい
  if (token->kind == TK_KEYWORD) {
    token = token->next;
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

  expect_punct(";");
  return node;
}

Node *expr() { Node *node = assign(); }

Node *assign() {
  Node *node = equality();
  if (consume_punct("=")) node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume_punct("=="))
      node = new_node(ND_EQ, node, add());
    else if (consume_punct("!="))
      node = new_node(ND_NE, node, add());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume_punct("<"))
      node = new_node(ND_LT, node, add());
    else if (consume_punct("<="))
      node = new_node(ND_LE, node, add());
    else if (consume_punct(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume_punct(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume_punct("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume_punct("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume_punct("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume_punct("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume_punct("+"))
    return primary();
  else if (consume_punct("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  else
    return primary();
}

Node *primary() {
  if (consume_punct("(")) {
    Node *node = expr();
    expect_punct(")");
    return node;
  }

  int offset = consume_ident();
  if (offset >= 0) return new_node_ident(offset);

  int val = expect_number();
  return new_node_num(val);
}
