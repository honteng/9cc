#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int pos = 0;
Token tokens[100];

Node *term();
Node *assign();
Node *expr();

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
  while (tokens[pos].ty == '*' || tokens[pos].ty == '/') {
    int ty = tokens[pos].ty;
    pos++;
    Node *rhs = term();
    lhs = new_node(ty, lhs, rhs);
  }

  return lhs;
}

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
  while (tokens[pos].ty == '+' || tokens[pos].ty == '-') {
    int ty = tokens[pos].ty;
    pos++;
    Node *rhs = mul();
    lhs = new_node(ty, lhs, rhs);
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
