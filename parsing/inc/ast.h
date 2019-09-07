#ifndef AST_PARSING
#define AST_PARSING

#include <bnf_parser.h>

typedef struct astnode {
  char* tag;
  void* content;
} ASTNode;

#endif
