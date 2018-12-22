#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Vector *code;

int pos = 0;

Node *term();
Node *assign();
Node *expr();
Node *expr_cmp();
Node *statement();
Node *if_statement();

Token* cur_token() {
  return (Token*)tokens->data[pos];
}

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
	Token *t0 = (Token*)tokens->data[0];
	Token *t = (Token*)tokens->data[pos];
	while (t->ty == '*' || t->ty == '/') {
		int ty = t->ty;
		pos++;
		Node *rhs = term();
		lhs = new_node(ty, lhs, rhs);
		t = (Token*)tokens->data[pos];
	}

	return lhs;
}

void program() {
  code = new_vector();

	Token *t = (Token*)tokens->data[pos];
	while (t->ty != TK_EOF) {
    Node* s = statement();
    if (s != NULL) {
      vec_push(code, s);
    }
    t = (Token*)tokens->data[pos];
	}
}

Node *statement() {
	Token *t = (Token*)tokens->data[pos];
  if (t->ty == ';') {
    pos++;
    return NULL;
  }

  if (t->ty == TK_RETURN) {
    pos++;
    Node *rhs = expr_cmp();
    Token *t = (Token*)tokens->data[pos];
    if (t->ty != ';') {
      char str[256];
      sprintf(str, "%d char 0x%x is not ;", __LINE__, t->ty);
      error(str);
    }
    pos++;
    return new_node(ND_RETURN, NULL, rhs);
  }

  if (t->ty == '{') {
    pos++;
    Node* block = new_node(ND_BLOCK, NULL, NULL);
    block->code = new_vector();
    t = (Token*)tokens->data[pos];
    while (t->ty != '}') {
      Node* s = statement();
      if (s != NULL) {
        vec_push(block->code, s);
      }
      t = (Token*)tokens->data[pos];
    }
    pos++;
    return block;
  }

  if (t->ty == TK_IF) {
    return if_statement();
  }

  Node *n = assign();
  return n;
}

/*
 * code[0] condition
 * lhs true block
 * rhs false block
 */
Node *if_statement() {
  pos++;
  Node* node = new_node(ND_IF, NULL, NULL);
  node->code = new_vector();
  Token *t = cur_token();
  if (t->ty != '(') {
		char str[256];
		sprintf(str, "%d char 0x%x is not (\n", __LINE__, t->ty);
		error(str);
  }
  pos++;
  Node *cmp = expr_cmp();
  vec_push(node->code, cmp);
  t = cur_token();
  if (t->ty != ')') {
		char str[256];
		sprintf(str, "%d char 0x%x is not )\n", __LINE__, t->ty);
		error(str);
  }
  pos++;
  node->lhs = statement();

  t = cur_token();
  if (t->ty == TK_ELSE) {
    pos++;
    node->rhs = statement();
  }
  return node;
}

/*
 * assign: expr_cmp assign' ";"
 * assign': eps | "=" expr_cmp assign'
 */
Node *assign() {
	Node *lhs = expr_cmp();
	Token *t = (Token*)tokens->data[pos];
	while (t->ty == '=') {
		pos++;
		lhs = new_node('=', lhs, expr_cmp());
		t = (Token*)tokens->data[pos];
	}
	if (t->ty != ';') {
		char str[256];
		sprintf(str, "%d char 0x%x is not ;", __LINE__, t->ty);
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
	Token *t = (Token*)tokens->data[pos];
	while (t->ty == TK_EQ || t->ty == TK_NEQ ||
      t->ty == TK_ST || t->ty == TK_STE ||
      t->ty == TK_GT || t->ty == TK_GTE) {
		int ty = t->ty;
		pos++;
		Node *rhs = expr();
		lhs = new_node(ty, lhs, rhs);
		t = (Token*)tokens->data[pos];
	}
	return lhs;
}

/* expr:  mul expr'
 * expr': eps | "+" expr | "-" expr
 */
Node *expr() {
	Node *lhs = mul();
	Token *t = (Token*)tokens->data[pos];
	while (t->ty == '+' || t->ty == '-') {
		int ty = t->ty;
		pos++;
		Node *rhs = mul();
		lhs = new_node(ty, lhs, rhs);
		t = (Token*)tokens->data[pos];
	}
	return lhs;
}

Node *term() {
	Token *t = (Token*)tokens->data[pos];
	if (t->ty == TK_NUM) {
		Node *n = new_node_num(t->val);
		pos++;
		return n;
	}
	if (t->ty == TK_IDENT) {
		Node *n = new_node_ident(*t->input);
		pos++;
		return n;
	}
	if (t->ty == '(') {
		pos++;
		Node *node = expr_cmp();
		t = (Token*)tokens->data[pos];
		if (t->ty != ')') {
			char str[256];
			sprintf(str, "char %s is not )", t->input);
			error(str);
		}
		pos++;
		return node;
	}
	char str[256];
	sprintf(str, "char %s is not supported", t->input);
	error(str);
}
