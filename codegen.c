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

void println(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  fprintf(stdout, "\n");
}

void gen_lval(Node *node) {
  if (node->kind == ND_LVAR) {
    println("  mov rax, rbp");
    println("  sub rax, %d", node->offset);
    println("  push rax");
  } else if (node->kind == ND_DEREF) {
    gen(node->lhs);
  } else {
    error("lvalue required as left operand of assignment");
  }
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_FUNDEF:
      println("%s:", node->fname);

      // Prologue
      // 変数26個分の領域を確保
      println("  push rbp");
      println("  mov rbp, rsp");
      println("  sub rsp, 208");

      // ABIのレジスタの値をoffsetを参照してスタックに設定
      for (int i = 0; node->args_offset[i] >= 0; i++) {
        printf("  mov [rbp-%d], %s\n", node->args_offset[i], argreg[i]);
      }

      gen(node->funcbody);

      // epilogue
      // デフォルト値(intなら0)を返り値に設定
      println("  mov rax, 0");
      println("  mov rsp, rbp");
      println("  pop rbp");
      println("  ret");
      return;
    case ND_VARDEF:
      // とりあえず何もしなくていい
      return;
    case ND_RETURN:
      gen(node->lhs);
      println("  pop rax");
      println("  mov rsp, rbp");
      println("  pop rbp");
      println("  ret");
      return;
    case ND_EXPR_STMT:
      gen(node->lhs);

      // exprの返り値をスタックから削除しておく
      println("  pop rax");
      return;
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
    case ND_DEREF:
      gen_lval(node);
      println("  pop rax");
      println("  mov rax, [rax]");
      println("  push rax");
      return;
    case ND_IF:  // 1
      gen(node->cond);
      println("  pop rax");
      println("  cmp rax, 0");
      println("  je .Lelse001");
      gen(node->then);
      println("  jmp .Lend001");
      println(".Lelse001:");
      if (node->els) gen(node->els);
      println(".Lend001:");
      return;
    case ND_FOR:  // 2
      if (node->init) gen(node->init);
      println(".Lbegin002:");
      gen(node->cond);
      println("  pop rax");
      println("  cmp rax, 0");
      println("  je .Lend002");
      gen(node->then);
      if (node->inc) gen(node->inc);
      println("  jmp .Lbegin002");
      println(".Lend002:");
      return;
    case ND_BLOCK:
      int i = 0;
      while (node->code[i]) {
        gen(node->code[i++]);
      }
      return;
    case ND_LVAR:
      gen_lval(node);
      println("  pop rax");
      println("  mov rax, [rax]");
      println("  push rax");
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
      println("  sub rsp, 8");
      println("  mov rax, rsp");
      println("  and rax, 15");
      println("  sub rsp, rax");

      printf("  call %s\n", node->fname);

      // 評価した値がつねにスタックトップに残るようにする
      println("  push rax");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      println("  pop rdi");
      println("  pop rax");
      println("  mov [rax], rdi");
      println("  push rdi");
      return;
  }

  // 以下は右左辺の区別がない二項演算

  gen(node->lhs);
  gen(node->rhs);

  println("  pop rdi");
  println("  pop rax");

  switch (node->kind) {
    case ND_ADD:
      println("  add rax, rdi");
      break;
    case ND_SUB:
      println("  sub rax, rdi");
      break;
    case ND_MUL:
      println("  imul rax, rdi");
      break;
    case ND_DIV:
      println("  cqo");
      println("  idiv rdi");
      break;
    case ND_RMD:
      println("  cqo");
      println("  idiv rdi");
      println("  mov rax, rdx");
      break;
    case ND_EQ:
      println("  cmp rax, rdi");
      println("  sete al");
      println("  movzb rax, al");
      break;
    case ND_NE:
      println("  cmp rax, rdi");
      println("  setne al");
      println("  movzb rax, al");
      break;
    case ND_LT:
      println("  cmp rax, rdi");
      println("  setl al");
      println("  movzb rax, al");
      break;
    case ND_LE:
      println("  cmp rax, rdi");
      println("  setle al");
      println("  movzb rax, al");
      break;
  }

  println("  push rax");
}