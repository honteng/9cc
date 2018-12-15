#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EOF,
};

enum {
  ND_NUM = 256,
  ND_IDENT,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

int pos = 0;
Token tokens[100];

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val; // used when ty == ND_NUM
  char name; // used when ty == ND_IDENT
} Node;

Node *code[100];

Node *term();

void error(char *str) {
  fprintf(stderr, "%s", str);
  exit(1);
}


void tokenize(char *p) {
  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '=' || *p == ';') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      p++;
      i++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      tokens[i].ty = TK_IDENT;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "unknown char %s in tokenizer", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(char name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

Node *mul() {
  Node *lhs = term();
  if (tokens[pos].ty == '*') {
    pos++;
    return new_node('*', lhs, mul());
  }
  if (tokens[pos].ty == '/') {
    pos++;
    return new_node('/', lhs, mul());
  }
  return lhs;
}

Node *assign();
Node *expr();

void program() {
  int i;
  i = 0;
  while (tokens[pos].ty != TK_EOF) {
    code[i] = assign();
    i++;
  }
  code[i] = NULL;
}

/*
 * assign: expr assign' ";"
 * assign': ε | "=" expr assign'
 */
Node *assign() {
  Node *lhs = expr();
  while (tokens[pos].ty == '=') {
    pos++;
    lhs = new_node('=', lhs, expr());
  }
  if (tokens[pos].ty != ';') {
    char str[256];
    sprintf(str, "char %s is not ;", tokens[pos].input);
    error(str);
  }
  pos++;
  return lhs;
}

/* expr:  mul expr'
 * expr': ε | "+" expr | "-" expr
 */
Node *expr() {
  Node *lhs = mul();
  if (tokens[pos].ty == '+') {
    pos++;
    return new_node('+', lhs, expr());
  }
  if (tokens[pos].ty == '-') {
    pos++;
    return new_node('-', lhs, expr());
  }
  return lhs;
}

Node *term() {
  if (tokens[pos].ty == TK_NUM) {
    return new_node_num(tokens[pos++].val);
  }
  if (tokens[pos].ty == TK_IDENT) {
    return new_node_ident(*tokens[pos++].input);
  }
  if (tokens[pos].ty == '(') {
    pos++;
    Node *node = expr();
    if (tokens[pos].ty != ')') {
      char str[256];
      sprintf(str, "char %s is not )", tokens[pos].input);
      error(str);
    }
    pos++;
    return node;
  }
  char str[256];
  sprintf(str, "char %s is not supported", tokens[pos].input);
  error(str);
}

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

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->ty) {
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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "arg# is not correct\n");
    return 1;
  }

  tokenize(argv[1]);
  //Node *node = expr();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 26*8\n");

  for (int i = 0; code[i] != NULL; i++) {
    gen(code[i]);
    printf("  pop rax\n");
  }

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
