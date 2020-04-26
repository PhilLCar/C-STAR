#ifndef AST_PARSING
#define AST_PARSING

#include <bnf_parser.h>
#include <generic_parser.h>
#include <symbol.h>
#include <strings.h>
#include <array.h>

#define AST_LOCK   0
#define AST_CLOSE -1

typedef enum aststatus {
  STATUS_POTENTIAL = 0,
  STATUS_CONFIRMED,
  STATUS_FAILED,
  STATUS_ONGOING,
  STATUS_NOSTATUS,
  STATUS_PARTIAL
} ASTStatus;

typedef struct astnode {
  String    *name;
  Array     *subnodes;
  String    *value;
  ASTStatus  status;
  int        pos;
} ASTNode;

ASTNode *parseast(char*);
void     deleteASTTree(ASTNode**);

#endif
