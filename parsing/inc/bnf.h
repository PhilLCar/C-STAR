#ifndef BNF_PARSING
#define BNF_PARSING

#include <string.h>

#include <symbol.h>
#include <terminal.h>
#include <error.h>
#include <array.h>
#include <strings.h>

#define INCLUDE_MAX_DEPTH   128
#define EBNF_TO_BNF         1

typedef enum bnftype {
  NODE_ROOT = 0,
  NODE_LEAF,
  NODE_CONCAT,
  NODE_LIST,
  NODE_ONE_OF,
  NODE_ONE_OR_NONE,
  NODE_MANY_OR_NONE,
  NODE_MANY_OR_ONE
} BNFType;

typedef struct bnfnode {
  char    *name;
  BNFType  type;
  void    *content;
  int      rec;
} BNFNode;

BNFNode *parsebnf(char*);
void     deleteBNFTree(BNFNode**);

#endif