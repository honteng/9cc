#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int if_cnt = 0;
int while_cnt = 0;

void gen_lval(Node *node) {
  if (node->ty == ND_IDENT) {
    printf("  mov rax, rbp\n");
    Map *local_env = (Map*)env->data[env->len-1];
    int idx = (int)map_get(local_env, node->long_name);
    if (idx == 0) {
      char str[256];
      sprintf(str, "var %s is not found", node->long_name);
      error(str);
    }
    printf("  sub rax, %d\n", idx * 8);
    printf("  push rax\n");
    return;
  }
  char str[256];
  sprintf(str, "char %c is ND_IDENT", node->ty);
  error(str);
}

int gen_statement(Node* node) {
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

  if (node->ty == ND_WHILE) {
    printf("WHILE%d:\n", while_cnt);
    gen((Node*)(node->code->data[0]));
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je ENDWHILE%d\n", if_cnt);
    gen(node->rhs);
    printf("  jmp WHILE%d\n", if_cnt);
    printf("ENDWHILE%d:\n", if_cnt);
    while_cnt++;
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

int gen_function(Node *node) {
  if (node->ty != ND_FUNC) {
    return 0;
  }

  // Make the function environment
  Map* local_env = new_map();
  vec_push(env, local_env);

  // start from 1 intentionally to check the var name is valid.
  int local_var = 1;

  printf("%s:\n", node->long_name);
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n"); // rbp has caller stack pointer
  for (int i = 0; i < node->params->len; i++) {
    Node *p = (Node*)node->params->data[i];
    if (p->ty != ND_IDENT) {
      error("Failed to get param");
    }
    printf("  mov rax, rbp\n");
    map_put(local_env, p->long_name, (void*)local_var);
    printf("  sub rax, %d\n", (local_var) * 8); // rax has a pointer to the value
    printf("  mov rdi, [rbp + %d]\n", i*8+16); // get the param value from the stack
    printf("  mov [rax], rdi\n"); // set the value to the stack
    local_var++;
  }

  // allocate auto variable area
  for (int i = 0; i < node->code->len; i++) {
    Node *d = (Node*)(node->code->data[i]);
    if (d->ty != ND_DECL_VAR) {
      error("Node is not decl var");
    }
    map_put(local_env, d->long_name, (void*)local_var);
    local_var++;
  }

  printf("  sub rsp, %d*8\n", local_var);

  node = node->rhs;
  if (node->ty != ND_BLOCK) {
    error("Node is not block");
  }
  for (int i = 0; i < node->code->len; i++) {
    Node *n = (Node*)node->code->data[i];
    gen(n);
  }

  vec_pop(env);

  return 1;
}

int gen_call_function(Node *node) {
  if (node->ty != ND_CALL_FUNC) {
    return 0;
  }

  for (int i = node->params->len-1; i >= 0; i--) {
    Node *p = (Node*)(node->params->data[i]);
    gen(p);
  }
  printf("  call %s\n", node->long_name);
  printf("  add rsp, %d\n", 8 * node->params->len);
  printf("  push rax\n");

  return 1;
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

  if (gen_statement(node)) {
    return;
  }
  if (gen_function(node)) {
    return;
  }
  if (gen_call_function(node)) {
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
    case '%':
      printf("  mov rdx, 0\n");
      printf("  div rdi\n");
      printf("  mov rax, rdx\n");
      break;
  }
  printf("  push rax\n");
}
