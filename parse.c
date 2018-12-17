#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int pos = 0;

Node *term();
Node *assign();
Node *expr();
Node *expr_cmp();

void error(char *str) {
  fprintf(stderr, "%s", str);
  exit(1);
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
 * assign: expr_cmp assign' ";"
 * assign': eps | "=" expr_cmp assign'
 */
Node *assign() {
  Node *lhs = expr_cmp();
  while (tokens[pos].ty == '=') {
    pos++;
    lhs = new_node('=', lhs, expr_cmp());
  }
  if (tokens[pos].ty != ';') {
    char str[256];
    sprintf(str, "char %s is not ;", tokens[pos].input);
    error(str);
  }
  pos++;
  return lhs;
}

/*
 * expr_cmp: exp (("==" | "!=") expr)*
 */
Node *expr_cmp() {
  Node *lhs = expr();
  while (tokens[pos].ty == TK_EQ || tokens[pos].ty == TK_NEQ) {
    int ty = tokens[pos].ty;
    pos++;
    Node *rhs = expr();
    lhs = new_node(ty, lhs, rhs);
  }
  return lhs;
}

/* expr:  mul expr'
 * expr': eps | "+" expr | "-" expr
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
    Node *node = expr_cmp();
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
