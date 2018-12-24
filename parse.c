#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Vector *code;
Map *functions;

int pos = 0;

Node *parse_term();
Node *parse_assign();
Node *parse_expr();
Node *parse_expr2();
Node *parse_statement();
Node *parse_if_statement();
Node *parse_while_statement();
Node *parse_function();

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

Node *parse_mul() {
	Node *lhs = parse_term();
	Token *t = cur_token();
	while (t->ty == '*' || t->ty == '/' || t->ty == '%') {
		int ty = t->ty;
		pos++;
		Node *rhs = parse_term();
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
      s = parse_statement();
    } else {
      s = parse_function();
    }
    if (s != NULL) {
      vec_push(code, s);
    }
    t = (Token*)tokens->data[pos];
	}
}

Node *parse_function() {
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
  n->rhs = parse_statement();
  t = cur_token();
  return n;
}

Node *parse_statement() {
	Token *t = cur_token();
  if (t->ty == ';') {
    pos++;
    return NULL;
  }

  if (t->ty == TK_RETURN) {
    pos++;
    Node *rhs = parse_expr();
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
      Node* s = parse_statement();
      if (s != NULL) {
        vec_push(block->code, s);
      }
      t = cur_token();
    }
    pos++;
    return block;
  }

  if (t->ty == TK_IF) {
    return parse_if_statement();
  }
  if (t->ty == TK_WHILE) {
    return parse_while_statement();
  }

  Node *n = parse_assign();
  return n;
}

/*
 * code[0] condition
 * lhs NULL
 * rhs block
 */
Node *parse_while_statement() {
  pos++;
  Node* node = new_node(ND_WHILE, NULL, NULL);
  node->code = new_vector();
  Token *t = cur_token();
  if (t->ty != '(') {
		char str[256];
		sprintf(str, "%d char 0x%x is not (\n", __LINE__, t->ty);
		error(str);
  }
  pos++;
  Node *cmp = parse_expr();
  vec_push(node->code, cmp);
  t = cur_token();
  if (t->ty != ')') {
		char str[256];
		sprintf(str, "%d char 0x%x is not )\n", __LINE__, t->ty);
		error(str);
  }
  pos++;
  node->rhs = parse_statement();
  return node;
}

/*
 * code[0] condition
 * lhs true block
 * rhs false block
 */
Node *parse_if_statement() {
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
  Node *cmp = parse_expr();
  vec_push(node->code, cmp);
  t = cur_token();
  if (t->ty != ')') {
		char str[256];
		sprintf(str, "%d char 0x%x is not )\n", __LINE__, t->ty);
		error(str);
  }
  pos++;
  node->lhs = parse_statement();

  t = cur_token();
  if (t->ty == TK_ELSE) {
    pos++;
    node->rhs = parse_statement();
  }
  return node;
}

/*
 * parse_assign: parse_expr parse_assign' ";"
 * parse_assign': eps | "=" parse_expr parse_assign'
 */
Node *parse_assign() {
	Node *lhs = parse_expr();
  Token *t = cur_token();
	while (t->ty == '=') {
		pos++;
		lhs = new_node('=', lhs, parse_expr());
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
 * parse_expr: exp (("==" | "!=") parse_expr2)*
 */
Node *parse_expr() {
	Node *lhs = parse_expr2();
  Token *t = cur_token();
	while (t->ty == TK_EQ || t->ty == TK_NEQ ||
      t->ty == TK_ST || t->ty == TK_STE ||
      t->ty == TK_GT || t->ty == TK_GTE) {
		int ty = t->ty;
		pos++;
		Node *rhs = parse_expr2();
		lhs = new_node(ty, lhs, rhs);
    t = cur_token();
	}
	return lhs;
}

/* parse_expr2:  parse_mul parse_expr2'
 * parse_expr2': eps | "+" parse_expr2 | "-" parse_expr2
 */
Node *parse_expr2() {
	Node *lhs = parse_mul();
  Token *t = cur_token();
	while (t->ty == '+' || t->ty == '-') {
		int ty = t->ty;
		pos++;
		Node *rhs = parse_mul();
		lhs = new_node(ty, lhs, rhs);
    t = cur_token();
	}
	return lhs;
}

Node *parse_term() {
  Token *t = cur_token();
	if (t->ty == TK_NUM) {
		Node *n = new_node_num(t->val);
		pos++;
		return n;
	}

	if (t->ty == TK_IDENT) {
    Token *pt = peek_token();

    // Parsing a call parse_function
    if (pt->ty == '(') {
      Node *f = map_get(functions, t->input);
      if (f == NULL) {
        char str[256]; sprintf(str, "parse_function %s is not found", t->input);
        error(str);
      }
      Node *n = new_node(ND_CALL_FUNC, NULL, NULL);
      n->long_name = t->input;

      pos++;
      pos++;
      t = cur_token();

      n->params = new_vector();
      while (t->ty != ')') {
        Node* p = parse_expr();
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
		Node *node = parse_expr();
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
