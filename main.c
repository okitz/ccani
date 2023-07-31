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

  gen(funcs[0]);

  return 0;
}