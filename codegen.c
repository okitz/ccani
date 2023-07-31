#include "ccani.h"

static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// エラーを投げる関数
// printfと同じインタフェース
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("lvalue required as left operand of assignment");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_IF:  // 1
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lelse001\n");
      gen(node->then);
      printf("  jmp .Lend001\n");
      printf(".Lelse001:\n");
      if (node->els) gen(node->els);
      printf(".Lend001:\n");
      return;
    case ND_FOR:  // 2
      if (node->init) gen(node->init);
      printf(".Lbegin002:\n");
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend002\n");
      gen(node->then);
      if (node->inc) gen(node->inc);
      printf("  jmp .Lbegin002\n");
      printf(".Lend002:\n");
      return;
    case ND_BLOCK:
      while (node->next) {
        node = node->next;
        gen(node);
      }
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_FUNCALL:
      Node *arg = node->next_arg;
      int arg_cnt = 0;
      while (arg) {
        gen(arg);
        arg_cnt++;
        arg = arg->next_arg;
      }
      for (int i = arg_cnt - 1; i >= 0; i--) {
        printf("  pop %s\n", argreg[i]);
      }

      // スタックのアラインメント
      // 参考:
      // https://github.com/hsjoihs/c-compiler/blob/c4dfc46ac9be116e6cbb8dd36f04ba55dd35a290/print_x86_64_unofficial.c#L185-L206
      printf("  sub rsp, 8\n");
      printf("  mov rax, rsp\n");
      printf("  and rax, 15\n");
      printf("  sub rsp, rax\n");

      printf("  call %s\n", node->fname);

      // 評価した値がつねにスタックトップに残るようにする
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
  }

  // 以下は右左辺の区別がない処理

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}