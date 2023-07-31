#include "ccani.h"

// program    = tpdef* func* glvar*
// tpdef*     = 保留
// glvar*     = 保留
// func       = "int" ident "(" (ident (","ident)*)? ")" "{" stmt* "}"
// stmt       = expr ";"
//            | "{" stmt* "}"
//            | "if" "(" expr ")" stmt ("else" stmt)?
//            | "while" "(" expr ")" stmt
//            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//            | "return" expr ";"
//            | "int" ident ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary = "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
// primary    = num | ident ("(" (expr (","expr)* )? ")")? | "(" expr ")"
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

// 次のトークンが識別子のときには読み進めてトークンを返す
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  else {
    Token *old_token = token;
    token = token->next;
    return old_token;
  }
}

// 次のトークンが識別子のときには読み進めてトークンを返す
Token *expect_ident() {
  if (token->kind != TK_IDENT)
    error_at(token->str, "expected identifier");
  else {
    Token *old_token = token;
    token = token->next;
    return old_token;
  }
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

// 次のトークンが期待するキーワードのときには読み進めて真を返す
bool consume_keyword(char *kw) {
  if (token->kind != TK_KEYWORD || strlen(kw) != token->len ||
      memcmp(token->str, kw, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待するキーワードのときには読み進める
void expect_keyword(char *kw) {
  if (token->kind != TK_KEYWORD || strlen(kw) != token->len ||
      memcmp(token->str, kw, token->len))
    error_at(token->str, "expected '%s'", kw);
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

Node *funcs[100];

void program() {
  int i = 0;
  while (!at_eof()) {
    funcs[i++] = func();
  }
  funcs[i] = NULL;
}

char *cut_ident_name(Token *tok) {
  char *name = calloc(tok->len + 1, sizeof(char));
  strncpy(name, tok->str, tok->len);
  name[tok->len] = '\x0';
  return name;
}

Node *func() {
  // 関数名
  Node *node = new_node(ND_FUNDEF);
  for (int i = 0; i < 6; i++) node->args_offset[i] = -1;

  expect_keyword("int");
  Token *tok = expect_ident();
  node->fname = cut_ident_name(tok);

  // 引数
  expect_punct("(");
  if (!consume_punct(")")) {
    for (int i = 0; i < 6; i++) {
      Token *arg = expect_ident();
      LVar *lvar = find_lvar(arg);
      if (!lvar) {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = arg->str;
        lvar->len = arg->len;
        lvar->offset = locals ? (locals->offset + 8) : 8;
        locals = lvar;
      }
      node->args_offset[i] = lvar->offset;

      if (consume_punct(")"))
        break;
      else
        expect_punct(",");
    }
  }

  expect_punct("{");
  int i = 0;
  while (!consume_punct("}")) {
    node->code[i++] = stmt();
  }
  node->code[i] = NULL;

  return node;
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
    } else if (consume_keyword("int")) {
      Token *tok = expect_ident();
      node = new_node(ND_VARDEF);
      LVar *lvar = find_lvar(tok);
      if (lvar) {
        error_at(tok->str, "%s is already defined", lvar->name);
      } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = cut_ident_name(tok);
        lvar->len = tok->len;
        lvar->offset = locals ? (locals->offset + 8) : 8;
        locals = lvar;
      }
      node->offset = lvar->offset;
    } else {
      node = expr();
    }
    expect_punct(";");
  }

  return node;
}

Node *expr() {
  Node *node = assign();
  return node;
}

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
    else if (consume_punct("%"))
      node = new_node_binary(ND_RMD, node, unary());
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
  } else if (consume_punct("&")) {
    return new_node_unitary(ND_ADDR, unary());
  } else if (consume_punct("*")) {
    return new_node_unitary(ND_DEREF, unary());
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

  Token *tok = consume_ident();
  if (tok) {
    if (consume_punct("(")) {
      node = new_node(ND_FUNCALL);
      node->fname = cut_ident_name(tok);

      Node *vector = node;
      if (!consume_punct(")")) {
        for (;;) {
          vector->next_arg = expr();
          vector = vector->next_arg;
          if (consume_punct(")"))
            break;
          else
            expect_punct(",");
        }
      }
    } else {
      node = new_node(ND_LVAR);
      LVar *lvar = find_lvar(tok);
      if (!lvar) {
        error_at(tok->str, "%s undeclared", lvar->name);
      }
      node->offset = lvar->offset;
    }
    return node;
  }

  node = new_node(ND_NUM);
  node->val = expect_number();
  return node;
}
