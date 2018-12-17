enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EQ,
  TK_NEQ,
  TK_EOF,
};

enum {
  ND_NUM = 256,
  ND_IDENT,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val; // used when ty == ND_NUM
  char name; // used when ty == ND_IDENT
} Node;

void tokenize(char *p);
void program();
void gen(Node *node);
void error(char *str);

extern Node *code[];
extern Token tokens[];
