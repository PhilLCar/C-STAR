#ifndef BNF_PARSING
#define BNF_PARSING

#include <string.h>

#include <diagnostic.h>
#include <symbol.h>
#include <terminal.h>
#include <error.h>
#include <array.h>
#include <strings.h>

#define INCLUDE_MAX_DEPTH   128
#define EBNF_TO_BNF         0

typedef enum bnfdef {
  BNF_NOT_DEFINED,
  BNF_DEFINED,
  BNF_DECLARED,
  BNF_DEFINED_AND_DECLARED
} BNFDef;

typedef enum bnftype {
  NODE_ROOT = 0,
  NODE_LEAF,
  NODE_RAW,
  NODE_CONCAT,
  NODE_LIST,
  NODE_REC,
  NODE_NOT,
  NODE_ANON,
  NODE_ONE_OF,
  NODE_ONE_OR_NONE,
  NODE_MANY_OR_NONE,
  NODE_MANY_OR_ONE
} BNFType;

typedef struct bnfnode {
  char    *name;
  void    *content;
  Array   *refs;
  BNFDef   def;
  BNFType  type;
  int      rec;
  int      order;
} BNFNode;

BNFNode *parsebnf(char*);
void     deleteBNFTree(BNFNode**);

#endif