#include "ccani.h"

// program    = stmt*
// stmt       = expr ";"
//            | "{" stmt* "}"
//            | "if" "(" expr ")" stmt ("else" stmt)?
//            | "while" "(" expr ")" stmt
//            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//            | "return" expr ";"
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

// 次のトークンが期待する記号pcのときには読み進めて真を返す
bool consume_punct(char *pc) {
  if (token->kind != TK_PUNCT || strlen(pc) != token->len ||
      memcmp(token->str, pc, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待する記号opのときには読み進める
void expect_punct(char *pc) {
  if (token->kind != TK_PUNCT || strlen(pc) != token->len ||
      memcmp(token->str, pc, token->len))
    error_at(token->str, "expected %s", pc);
  token = token->next;
}

bool consume_keyword(char *kw) {
  if (token->kind != TK_KEYWORD || strlen(kw) != token->len ||
      memcmp(token->str, kw, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが数値の場合、読み進めて数値を返す
int expect_number() {
  if (token->kind != TK_NUM) error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_unitary(NodeKind kind, Node *lhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
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

  if (consume_keyword("if")) {
    node = new_node(ND_IF);
    expect_punct("(");
    node->cond = expr();
    expect_punct(")");
    node->then = stmt();
    if (consume_keyword("else")) {
      node->els = stmt();
    }
  } else if (consume_keyword("while")) {
    node = new_node(ND_FOR);
    expect_punct("(");
    node->cond = expr();
    expect_punct(")");
    node->then = stmt();

  } else if (consume_keyword("for")) {
    node = new_node(ND_FOR);
    expect_punct("(");
    node->init = expr();
    expect_punct(";");
    node->cond = expr();
    expect_punct(";");
    node->inc = expr();
    expect_punct(")");
    node->then = stmt();
  } else if (consume_punct("{")) {
    node = new_node(ND_BLOCK);
    Node *vector = node;
    while (!consume_punct("}")) {
      vector->next = stmt();
      vector = vector->next;
    }
  } else {  // 末尾に ; が必要な文
    if (consume_keyword("return")) {
      node = new_node_unitary(ND_RETURN, expr());
    } else {
      node = expr();
    }
    expect_punct(";");
  }

  return node;
}

Node *expr() { Node *node = assign(); }

Node *assign() {
  Node *node = equality();
  if (consume_punct("=")) node = new_node_binary(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume_punct("=="))
      node = new_node_binary(ND_EQ, node, add());
    else if (consume_punct("!="))
      node = new_node_binary(ND_NE, node, add());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume_punct("<"))
      node = new_node_binary(ND_LT, node, add());
    else if (consume_punct("<="))
      node = new_node_binary(ND_LE, node, add());
    else if (consume_punct(">"))
      node = new_node_binary(ND_LT, add(), node);
    else if (consume_punct(">="))
      node = new_node_binary(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume_punct("+"))
      node = new_node_binary(ND_ADD, node, mul());
    else if (consume_punct("-"))
      node = new_node_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume_punct("*"))
      node = new_node_binary(ND_MUL, node, unary());
    else if (consume_punct("/"))
      node = new_node_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume_punct("+"))
    return primary();
  else if (consume_punct("-")) {
    Node *zero_node = new_node(ND_NUM);
    zero_node->val = 0;
    return new_node_binary(ND_SUB, zero_node, primary());
  } else
    return primary();
}

Node *primary() {
  Node *node;

  if (consume_punct("(")) {
    node = expr();
    expect_punct(")");
    return node;
  }

  int offset = consume_ident();
  if (offset >= 0) {
    node = new_node(ND_LVAR);
    node->offset = offset;
    return node;
  }

  node = new_node(ND_NUM);
  node->val = expect_number();
  return node;
}
