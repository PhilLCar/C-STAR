#ifndef BNF_PARSING
#define BNF_PARSING

#include <string.h>

#include "symbol.h"

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
  void *nodelist;
} Node;

#endif
