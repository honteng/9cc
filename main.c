#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

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
    printf("  sub rsp, 26*8\n");

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

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
//    printf("main:\n");
//
//    // prologue
//    printf("  push rbp\n");
//    printf("  mov rbp, rsp\n");
//    printf("  sub rsp, 26*8\n");
//
//    // call hoge for now
//
//    printf("  push 1\n");
//    printf("  push 2\n");
//    printf("  call hoge\n");
//
//    // epilogue
//    printf("  mov rsp, rbp\n");
//    printf("  pop rbp\n");
//    printf("  ret\n");

    fprintf(stderr, "#code %d\n", code->len);
    for (int i = 0; i < code->len; i++) {
      gen((Node*)code->data[i]);
      printf("  pop rax\n");
    }

  }
	return 0;
}
