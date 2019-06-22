#ifndef BNF_PARSING
#define BNF_PARSING

#include <string.h>

#include <symbol.h>
#include <error.h>
#include <terminal.h>

enum type {
  NODE_ROOT = 0,
  NODE_LEAF,
  NODE_LIST,
  NODE_ONE_OF,
  NODE_ONE_OR_NONE,
  NODE_MANY_OR_NONE,
  NODE_UNKNOWN
};

typedef struct node {
  char *name;
  type  type;
  void *content;
  int   num;
  int   cap;
} Node;

Node  newNode(Node*, char*, type);
void  deleteNode(Node**);
Node  getnode(Node*, char*);
void  addnode(Node*, Node*);
int   parsenode(Node*, Node*, SymbolStream*, char*);
void  link(Node*);
Node *parsefile(char*);
int   parseinclude(Node*, SymbolStream*);
int   parseline(Node*, SymbolStream*);
int   expect(SymbolStream*, char*);

#endif
