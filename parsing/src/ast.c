#include <ast.h>

ASTNode *astfrombnf(BNFNode *bnf)
{
  ASTNode *ast = malloc(sizeof(ASTNode));
  if (ast) {
    if (bnf->type == NODE_LEAF) {
      ast->name = newString(bnf->content);
    } else {
      ast->name = newString(bnf->name);
    }
    ast->subnodes = newArray(sizeof(ASTNode));
    ast->value    = newString("");
    ast->status   = STATUS_NOSTATUS;
    ast->ref      = bnf;
    ast->pos      = 0;
  }
  return ast;
}

ASTNode *initast(BNFNode *root)
{
  Array *rnodes = root->content;
  if (rnodes->size != 1) {
    char error[256];
    sprintf(error, "The BNF tree described by node <"
                   FONT_BOLD"%s"FONT_RESET"> is unsuitable for producing an AST."
                   " (The root must have a single child)", root->name);
    printmessage(ERROR, error);
  }
  return NULL;
}

ASTNode *newcharast(ASTNode *ast, BNFNode *bnf, char c)
{
  ASTNode *current;
  if (bnf->type == NODE_LEAF) {

  } else if (bnf->type == NODE_LEAF_CONCAT) {

  } else if (bnf->type == NODE_ONE_OF) {
    // always check
  } else if (bnf->type == NODE_LIST) {
    // take the first AST node that is not CONFIRMED
    // continue recursion
    if (!current->subnodes->size) {
      
    }
    for (int i = 0; i < current->subnodes->size; i++) {
      if (((ASTNode*)at(current->subnodes, i))->status == STATUS_ONGOING) {
        
      }
    }
  } else if (bnf->type == NODE_ONE_OR_NONE) {
    // if node is null check, if node is CONFIRMED
    // mark topnode as fail
  } else if (bnf->type == NODE_MANY_OR_NONE) {
    // always check
  }

  return NULL;
}


ASTNode *parseast(char *filename)
{
  BNFNode      *root   = parsebnf("parsing/bnf/test.bnf");
  Parser       *parser = newParser("parsing/prs/csr.prs");
  SymbolStream *ss     = ssopen(filename, parser);
  ASTNode      *ast    = initast(root);

  Symbol *s;
  while (!(s = ssgets(ss))->eof) {
    printf("%s\n", s->text);
  }

  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&root);
  return NULL;
}