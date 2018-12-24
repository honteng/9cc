#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Vector *code;
Map *functions;

int pos = 0;

Node *term();
Node *assign();
Node *expr();
Node *expr_cmp();
Node *statement();
Node *if_statement();
Node *function();

Token* cur_token() {
  return (Token*)tokens->data[pos];
}

Token* peek_token() {
  return (Token*)tokens->data[pos+1];
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
	Token *t = cur_token();
	while (t->ty == '*' || t->ty == '/') {
		int ty = t->ty;
		pos++;
		Node *rhs = term();
		lhs = new_node(ty, lhs, rhs);
		t = cur_token();
	}

	return lhs;
}

void program(int simple) {
  code = new_vector();
  functions = new_map();

	Token *t = cur_token();
	while (t->ty != TK_EOF) {
    Node* s;
    if (simple) {
      s = statement();
    } else {
      s = function();
    }
    if (s != NULL) {
      vec_push(code, s);
    }
    t = (Token*)tokens->data[pos];
	}
}

Node *function() {
  Node *n = new_node(ND_FUNC, NULL, NULL);

	Token *t = cur_token();
  if (t->ty != TK_IDENT) {
    char str[256];
    sprintf(str, "%d char 0x%x is not IDENT\n", __LINE__, t->ty);
    error(str);
  }

  n->long_name = t->input;
  map_put(functions, n->long_name, n);

  pos++;
  t = cur_token();
  if (t->ty != '(') {
    char str[256];
    sprintf(str, "%d char 0x%x is not IDENT\n", __LINE__, t->ty);
    error(str);
  }

  // Parse params
  pos++;
  t = cur_token();
  n->params = new_vector();
  while (t->ty != ')') {
    if (t->ty != TK_IDENT) {
      error("Failed to parse func params0");
    }
    vec_push(n->params, new_node_ident(*t->input));
    pos++;
    t = cur_token();
    if (t->ty == ',') {
      pos++;
      t = cur_token();
    } else if(t->ty != ')') {
      error("Failed to parse func params1");
    }
  }

  // Parse func body
  pos++;
  t = cur_token();
  if (t->ty != '{') {
    error("Failed to parse func body0");
  }
  n->rhs = statement();
  t = cur_token();
  return n;
}

Node *statement() {
	Token *t = cur_token();
  if (t->ty == ';') {
    pos++;
    return NULL;
  }

  if (t->ty == TK_RETURN) {
    pos++;
    Node *rhs = expr_cmp();
    t = cur_token();
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
    t = cur_token();
    while (t->ty != '}') {
      Node* s = statement();
      if (s != NULL) {
        vec_push(block->code, s);
      }
      t = cur_token();
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
  Token *t = cur_token();
	while (t->ty == '=') {
		pos++;
		lhs = new_node('=', lhs, expr_cmp());
    t = cur_token();
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
  Token *t = cur_token();
	while (t->ty == TK_EQ || t->ty == TK_NEQ ||
      t->ty == TK_ST || t->ty == TK_STE ||
      t->ty == TK_GT || t->ty == TK_GTE) {
		int ty = t->ty;
		pos++;
		Node *rhs = expr();
		lhs = new_node(ty, lhs, rhs);
    t = cur_token();
	}
	return lhs;
}

/* expr:  mul expr'
 * expr': eps | "+" expr | "-" expr
 */
Node *expr() {
	Node *lhs = mul();
  Token *t = cur_token();
	while (t->ty == '+' || t->ty == '-') {
		int ty = t->ty;
		pos++;
		Node *rhs = mul();
		lhs = new_node(ty, lhs, rhs);
    t = cur_token();
	}
	return lhs;
}

Node *term() {
  Token *t = cur_token();
	if (t->ty == TK_NUM) {
		Node *n = new_node_num(t->val);
		pos++;
		return n;
	}

	if (t->ty == TK_IDENT) {
    Token *pt = peek_token();

    // Parsing a call function
    if (pt->ty == '(') {
      Node *f = map_get(functions, t->input);
      if (f == NULL) {
        char str[256]; sprintf(str, "function %s is not found", t->input);
        error(str);
      }
      Node *n = new_node(ND_CALL_FUNC, NULL, NULL);
      n->long_name = t->input;

      pos++;
      pos++;
      t = cur_token();

      n->params = new_vector();
      while (t->ty != ')') {
        Node* p = expr_cmp();
        vec_push(n->params, p);
        t = cur_token();
        if (t->ty == ',') {
          pos++;
          t = cur_token();
        } else if(t->ty != ')') {
          error("Failed to parse func params1");
        }
      }
      pos++;
      return n;
    }

    // Refer to a variable
		Node *n = new_node_ident(*t->input);
		pos++;
		return n;
	}

	if (t->ty == '(') {
		pos++;
		Node *node = expr_cmp();
    t = cur_token();
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
