#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int if_cnt = 0;

void gen_lval(Node *node) {
  if (node->ty == ND_IDENT) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", ('z' - node->name + 1) * 8);
    printf("  push rax\n");
    return;
  }
  char str[256];
  sprintf(str, "char %c is ND_IDENT", node->ty);
  error(str);
}

int statement_gen(Node* node) {
  if (node->ty == ND_RETURN) {
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 1;
  }

  if (node->ty == ND_BLOCK) {
    for (int i = 0; i < node->code->len; i++) {
      Node *n = (Node*)node->code->data[i];
      gen(n);
    }
    return 1;
  }

  if (node->ty == ND_IF) {
    gen((Node*)(node->code->data[0]));
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je ELSE%d\n", if_cnt);
    gen(node->lhs);
    printf("  je ENDIF%d\n", if_cnt);
    printf("ELSE%d:\n", if_cnt);
    if (node->rhs) {
      gen(node->rhs);
    }
    printf("ENDIF%d:\n", if_cnt);
    if_cnt++;
    return 1;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return 1;
  }

  return 0;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (statement_gen(node)) {
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->ty) {
    case TK_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case TK_NEQ:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case TK_ST:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case TK_STE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case TK_GT:
      printf("  cmp rax, rdi\n");
      printf("  setg al\n");
      printf("  movzb rax, al\n");
      break;
    case TK_GTE:
      printf("  cmp rax, rdi\n");
      printf("  setge al\n");
      printf("  movzb rax, al\n");
      break;
    case '+':
      printf("  add rax, rdi\n");
      break;
    case '-':
      printf("  sub rax, rdi\n");
      break;
    case '*':
      printf("  mul rdi\n");
      break;
    case '/':
      printf("  mov rdx, 0\n");
      printf("  div rdi\n");
      break;
  }
  printf("  push rax\n");
}
