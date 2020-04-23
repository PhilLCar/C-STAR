#ifndef BNF_PARSING
#define BNF_PARSING

#include <string.h>

#include <symbol.h>
#include <terminal.h>
#include <error.h>
#include <array.h>

#define INCLUDE_MAX_DEPTH   128
#define NODENAME_MAX_LENGTH 256

typedef enum nodetype {
  NODE_ROOT = 0,
  NODE_LEAF,
  NODE_LIST,
  NODE_ONE_OF,
  NODE_ONE_OR_NONE,
  NODE_MANY_OR_NONE
} Type;

typedef struct node {
  char *name;
  Type  type;
  void *content;
} Node;

#endif