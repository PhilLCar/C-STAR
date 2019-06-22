#ifndef BNF_PARSING
#define BNF_PARSING

#include <string.h>

#include <symbol.h>
#include <error.h>

enum type {
  NODE_LEAF = 0,
  NODE_LIST,
  NODE_ONE_OF,
  NODE_ONE_OR_NONE,
  NODE_MANY_OR_NONE
};

typedef struct node {
  char *name;
  int   type;
  Node *nodelist;
  int   num;
  int   cap;
} Node;

Node *parsefile(char*);
void  parseincludes(Node*, SymbolStream*);
void  parseline(Node*, SymbolStream*);
int   expect(SymbolStream*, char*);

#endif
