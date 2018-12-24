#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Vector *env;

int main(int argc, char **argv) {
	if (argc != 2 && argc != 3) {
		fprintf(stderr, "arg# is not correct\n");
		return 1;
	}

	if (strcmp("-test", argv[1]) == 0) {
		runtest();
		return 0;
	}

	if (strcmp("-with_main", argv[1]) == 0) {
    tokenize(argv[2]);
    program(1);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    printf("main:\n");

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    // allocate local a-z values virtually here
    Map* local_env = new_map();
    // start from 1 intentionally to check the var name is valid.
    int local_var = 1;
    env = new_vector();
    vec_push(env, local_env);
    for (char c = 'a'; c <= 'z'; c++) {
      char* k = strndup(&c, 1);
      map_put(local_env, k, (void*)local_var);
      local_var++;
    }
    printf("  sub rsp, %d*8\n", local_var);

    for (int i = 0; i < code->len; i++) {
      gen((Node*)code->data[i]);
      printf("  pop rax\n");
    }

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  } else {
    tokenize(argv[1]);
    program(0);
    env = new_vector();
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    for (int i = 0; i < code->len; i++) {
      gen((Node*)code->data[i]);
      printf("  pop rax\n");
    }

  }
	return 0;
}
