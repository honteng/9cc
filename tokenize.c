#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Vector *tokens = NULL;
Map *keywords = NULL;

void dump() {
  fprintf(stderr, "====\n");
  for (int i = 0; i < tokens->len; i++) {
    Token *t = (Token*)tokens->data[i];
    fprintf(stderr, "0x%x\n", t->ty);
  }
}

int isletter(char c) {
  return ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') || c == '_';
}

void setup_keywords() {
  keywords = new_map();
  map_put(keywords, "return", (void*)TK_RETURN);
  map_put(keywords, "if", (void*)TK_IF);
  map_put(keywords, "else", (void*)TK_ELSE);
  map_put(keywords, "for", (void*)TK_FOR);
  map_put(keywords, "while", (void*)TK_WHILE);
  map_put(keywords, "char", (void*)TK_CHAR);
  map_put(keywords, "int", (void*)TK_INT);
}

int lookup_ident(char *ident) {
  int val = (int)map_get(keywords, ident);
  if (val != 0) {
    return val;
  }
  return TK_IDENT;
}

void tokenize(char *p) {
  tokens = new_vector();
  setup_keywords();

  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '=' && *(p+1) == '=') {
      Token *t = (Token*)malloc(sizeof(Token));
      t->ty = TK_EQ;
      t->input = p;
      vec_push(tokens, t);
      p += 2;
      continue;
    }

    if (*p == '!' && *(p+1) == '=') {
      Token *t = (Token*)malloc(sizeof(Token));
      t->ty = TK_NEQ;
      t->input = p;
      vec_push(tokens, t);
      p += 2;
      continue;
    }

    if (*p == '<') {
      Token *t = (Token*)malloc(sizeof(Token));
      if (*(p+1) == '=') {
        t->ty = TK_STE;
        t->input = p;
        vec_push(tokens, t);
        p += 2;
        continue;
      }
      t->ty = TK_ST;
      t->input = p;
      vec_push(tokens, t);
      p++;
      continue;
    }

    if (*p == '>') {
      Token *t = (Token*)malloc(sizeof(Token));
      if (*(p+1) == '=') {
        t->ty = TK_GTE;
        t->input = p;
        vec_push(tokens, t);
        p += 2;
        continue;
      }
      t->ty = TK_GT;
      t->input = p;
      vec_push(tokens, t);
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '=' || *p == ';' ||
        *p == ',' || *p == '{' || *p == '}') {
      Token *t = (Token*)malloc(sizeof(Token));
      t->ty = *p;
      t->input = p;
      vec_push(tokens, t);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      Token *t = (Token*)malloc(sizeof(Token));
      t->ty = TK_NUM;
      t->input = p;
      t->val = strtol(p, &p, 10);
      vec_push(tokens, t);
      continue;
    }

    if (isletter(*p)) {
      Token *t = (Token*)malloc(sizeof(Token));
      t->input = p;
      while(isletter(*p)) {
        p++;
      }

      int l = p - t->input;
      char* s = strndup(t->input, l);
      t->ty = lookup_ident(s);
      t->input = s;
      vec_push(tokens, t);
      continue;
    }

    fprintf(stderr, "unknown char %s in tokenizer", p);
    exit(1);
  }

  Token *t = (Token*)malloc(sizeof(Token));
  t->ty = TK_EOF;
  t->input = p;
  vec_push(tokens, t);
}
