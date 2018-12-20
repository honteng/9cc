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
}

