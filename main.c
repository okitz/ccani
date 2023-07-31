#include "ccani.h"

// 入力プログラム
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments", argv[0]);
  }

  user_input = argv[1];
  tokenize();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  // 逐次的に関数を生成
  for (int i = 0; funcs[i]; i++) {
    gen(funcs[i]);
  }

  return 0;
}