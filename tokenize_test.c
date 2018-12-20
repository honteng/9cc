#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

extern void expect(int line, int expected, int actual);

void test_tokenize() {
  tokenize("{};");
  expect(__LINE__, tokens->len, 4);
  tokenize(" \t\n;");
  expect(__LINE__, tokens->len, 2);
  tokenize(" aa;");
  expect(__LINE__, tokens->len, 3);

  tokenize("<");
  Token *t = (Token*)tokens->data[0];
  expect(__LINE__, t->ty, TK_ST);
  tokenize("<=");
  t = (Token*)tokens->data[0];
  expect(__LINE__, t->ty, TK_STE);

  tokenize(">");
  t = (Token*)tokens->data[0];
  expect(__LINE__, t->ty, TK_GT);
  tokenize(">=");
  t = (Token*)tokens->data[0];
  expect(__LINE__, t->ty, TK_GTE);
}

