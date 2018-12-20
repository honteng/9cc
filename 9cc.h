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
  int input_len;
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

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

extern Vector *tokens; // *Token
extern Vector *code; // *Node

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

void runtest();
