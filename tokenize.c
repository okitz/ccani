#include "ccani.h"

// 現在着目しているトークン
Token *token;

void error_at(char *loc, char *fmt, ...) {
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

struct Keyword {
  char *keyword;
  int len;
};

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

bool isVaildVarChar(char c) {
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || (c == '_') ||
         ('a' <= c && c <= 'z');
}

// *pがキーワードなら長さを返す
#define KWN 5
int isKeyword(char *p) {
  const struct Keyword KEYWORDS[KWN] = {
      {"return", 6}, {"if", 2}, {"while", 5}, {"for", 3}, {"else", 4}};
  int len = 0;
  for (int i = 0; i < KWN; i++) {
    char *kw_str = KEYWORDS[i].keyword;
    int kw_len = KEYWORDS[i].len;
    if (startswith(p, kw_str) && !isVaildVarChar(p[kw_len])) {
      len = kw_len;
      break;
    }
  }
  return len;
}

// 入力文字列pをトークナイズ
void tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
      cur = new_token(TK_PUNCT, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/%()><={};,", *p)) {
      cur = new_token(TK_PUNCT, cur, p++, 1);
      continue;
    }

    int kw_len = isKeyword(p);
    if (kw_len) {
      cur = new_token(TK_KEYWORD, cur, p, kw_len);
      p += kw_len;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, -1);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if (isVaildVarChar(*p) && !('0' <= *p && *p <= '9')) {
      int len = 1;
      while (isVaildVarChar(p[len])) len++;
      cur = new_token(TK_IDENT, cur, p, len);
      p += len;
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  token = head.next;
}
