#ifndef AST_PARSING
#define AST_PARSING

#include <diagnostic.h>
#include <bnf.h>
#include <array.h>
#include <symbol.h>

typedef enum aststatus {
  STATUS_FAILED = 0,
  STATUS_CONFIRMED,
  STATUS_PARTIAL
} ASTStatus;

typedef enum astflags {
  ASTFLAGS_NONE       = 0,
  ASTFLAGS_NO_NEWLINE = 1
} ASTFlags;

typedef struct astnode {
  String  *name;
  String  *value;
  BNFNode *ref;
  Symbol  *symbol;
  Array   *subnodes;
  int      continuation;
  int      recurse;
  int      scope;
} ASTNode;

ASTNode   *newASTNode(ASTNode *ast, BNFNode *bnf);
void       deleteAST(ASTNode **ast);
ASTStatus  astparsestream(ASTNode *ast, BNFNode *bnf, Stream *stream);
ASTNode   *parseast(char *filename);

#endif
