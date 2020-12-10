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

typedef struct astnode {
  String  *name;
  String  *value;
  BNFNode *ref;
  Symbol  *symbol;
  Array   *subnodes;
  int      scope;
  char     continuation;
  char     recurse;
  char     newline;
} ASTNode;

ASTNode   *newASTNode(ASTNode *ast, BNFNode *bnf);
void       deleteAST(ASTNode **ast);
ASTStatus  astparsestream(ASTNode *ast, BNFNode *bnf, Stream *stream);
ASTNode   *parseast(char *filename);

#endif
