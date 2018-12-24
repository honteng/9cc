enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EQ,
  TK_NEQ,
  TK_ST, // Smaller Then
  TK_STE, // Smaller Then or Equal to
  TK_GT, // Greater Then
  TK_GTE, // Greater Then or Equal to
  TK_IF,
  TK_ELSE,
  TK_RETURN,
  TK_FOR,
  TK_WHILE,
  TK_INT,
  TK_CHAR,
  TK_EOF,
};

// For now, matching to TK_
enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_EQ,
  ND_NEQ,
  ND_ST, // Smaller Then
  ND_STE, // Smaller Then or Equal to
  ND_GT, // Greater Then
  ND_GTE, // Greater Then or Equal to
  ND_IF,
  ND_ELSE, // Should not be used
  ND_RETURN,
  ND_FOR,
  ND_WHILE,
  ND_INT,
  ND_CHAR,
  ND_EOF,

  ND_BLOCK = 512,
  ND_FUNC,
  ND_CALL_FUNC,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

void tokenize(char *p);
void program();
void error(char *str);

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct Node {
  int ty;
  char *long_name;
  Vector *params; // *Node used when ND_FUNC
  Vector *code; // *Node
  struct Node *lhs;
  struct Node *rhs;
  int val; // used when ty == ND_NUM
  char name; // used when ty == ND_IDENT
} Node;

void gen(Node *node);

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
