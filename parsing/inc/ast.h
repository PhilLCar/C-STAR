#ifndef AST_PARSING
#define AST_PARSING

#include <diagnostic.h>
#include <bnf.h>
#include <generic_parser.h>
#include <symbol.h>
#include <strings.h>
#include <array.h>

typedef enum aststatus {
  STATUS_FAILED = 0,
  STATUS_CONFIRMED,
  STATUS_PARTIAL
} ASTStatus;

typedef enum astflags {
  ASTFLAGS_NONE   = 0,
  ASTFLAGS_REC    = 2,
  ASTFLAGS_FRONT  = 4,
  ASTFLAGS_RECLVL = ~0xFF
} ASTFlags;

typedef struct astnode {
  String  *name;
  String  *value;
  BNFNode *ref;
  Symbol  *symbol;
  Array   *subnodes;
  int      continuation;
  int      recurse;
} ASTNode;

ASTNode   *newASTNode(ASTNode*, BNFNode*);
void       deleteAST(ASTNode**);
ASTStatus  astparsestream(ASTNode*, BNFNode*, Array*, ASTFlags, Stream*);
ASTNode   *parseast(char*);

#endif
