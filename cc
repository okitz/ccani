.intel_syntax noprefix
.globl main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 208
  push 90
  push 144
  pop rsi
  pop rdi
  sub rsp, 8
  mov rax, rsp
  and rax, 15
  sub rsp, rax
  call gcd
  push rax
  pop rax
  mov rsp, rbp
  pop rbp
  ret
gcd:
  push rbp
  mov rbp, rsp
  sub rsp, 208
  mov [rbp-8], rdi
  mov [rbp-16], rsi
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  push 0
  pop rdi
  pop rax
  cmp rax, rdi
  sete al
  movzb rax, al
  push rax
  pop rax
  cmp rax, 0
  je .Lelse001
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rax
  mov rsp, rbp
  pop rbp
  ret
  push 0
  jmp .Lend001
.Lelse001:
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rdi
  pop rax
  cqo
  idiv rdi
  mov rax, rdx
  push rax
  pop rsi
  pop rdi
  sub rsp, 8
  mov rax, rsp
  and rax, 15
  sub rsp, rax
  call gcd
  push rax
  pop rax
  mov rsp, rbp
  pop rbp
  ret
  push 0
.Lend001:
  pop rax
  mov rsp, rbp
  pop rbp
  ret
