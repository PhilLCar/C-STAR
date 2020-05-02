#ifndef AST_PARSING
#define AST_PARSING

#include <diagnostic.h>
#include <bnf.h>
#include <generic_parser.h>
#include <symbol.h>
#include <strings.h>
#include <array.h>

#define AST_CONCAT ' '
#define AST_LOCK    0
#define AST_CLOSE  -1

typedef enum aststatus {
  STATUS_NOSTATUS = 0,
  STATUS_ONGOING,
  STATUS_CONFIRMED,
  STATUS_FAILED,
  STATUS_PARTIAL
} ASTStatus;

typedef enum asterrortype {
  ERROR_CONCAT_0MATCH,
  ERROR_CONCAT_MANYMATCH,
  ERROR_UNIMPLEMENTED,
  WARNING_AMBIGUOUS
} ASTErrorType;

typedef struct asterror {
  ASTErrorType  errno;
  BNFNode      *bnfref;
} ASTError;

typedef struct astnode {
  String         *name;
  BNFNode        *ref;
  Array          *subnodes;
  String         *value;
  ASTStatus       status;
  int             pos;
} ASTNode;

ASTNode *parseast(char*);
void     deleteAST(ASTNode**);

#endif
