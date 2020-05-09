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
#define EBNF_TO_BNF         1

typedef enum bnftype {
  NODE_ROOT = 0,
  NODE_LEAF,
  NODE_RAW,
  NODE_CONCAT,
  NODE_LIST,
  NODE_REC,
  NODE_ONE_OF,
  NODE_ONE_OR_NONE,
  NODE_MANY_OR_NONE,
  NODE_MANY_OR_ONE
} BNFType;

typedef struct bnfnode {
  char    *name;
  int      def;
  BNFType  type;
  void    *content;
  int      rec;
  Array   *refs;
} BNFNode;

BNFNode *parsebnf(char*);
void     deleteBNFTree(BNFNode**);

#endif