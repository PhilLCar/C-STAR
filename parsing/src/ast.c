#include <ast.h>

ASTNode *astfrombnf(BNFNode *bnf)
{
  ASTNode *ast = malloc(sizeof(ASTNode));
  if (ast) {
    ast->name     = newString(bnf->name);
    ast->subnodes = newArray(sizeof(ASTNode));
    ast->value    = newString("");
    ast->status   = STATUS_NOSTATUS;
    ast->pos      = 0;
  }
  return ast;
}

void freeastnode(ASTNode *node) {
  deleteString(&node->name);
  deleteString(&node->value);
  deleteArray(&node->subnodes);
}

void concatastnode(ASTNode *a, ASTNode *b) {
  for (int i = 0; i < b->subnodes->size; i++) {
    concatastnode(b, at(b->subnodes, i));
  }
  concat(a->value, newString(b->value->content));
}

void pushastnode(ASTNode *node, ASTNode **ref) {
  push(node->subnodes, *ref);
  free(*ref);
  *ref = at(node->subnodes, node->subnodes->size - 1);
}

void astswaparray(ASTNode *super, ASTNode *sub) {
  Array *tmp = sub->subnodes;
  sub->subnodes = NULL;
  while (super->subnodes->size) freeastnode(pop(super->subnodes));
  deleteArray(&super->subnodes);
  super->subnodes = tmp;
}

void newcharast(Array *reserved, ASTNode *ast, BNFNode *bnf, int con, char c)
{
  ASTNode *subnode;
  ASTNode *supernode;
  BNFNode *subbnf;
  char    *content;
  int      waspartial;

  switch (bnf->type) {
    case NODE_ROOT:
      break;
    //////////////////////////////////////////////////////////////////////////
    case NODE_CONCAT:
      if (ast->pos < ((Array*)bnf->content)->size) {
        subnode = at(ast->subnodes, ast->pos);
        subbnf  = at(bnf->content,  ast->pos);
        if (!subnode) {
          subnode = astfrombnf(subbnf);
          subnode->status = STATUS_NOSTATUS;
          pushastnode(ast, &subnode);
        }
        waspartial = subnode->status == STATUS_PARTIAL;
        newcharast(reserved, subnode, subbnf, 1, c);
        if (subnode->status == STATUS_PARTIAL) {
          
        } else if (subnode->status == STATUS_FAILED) {
          ast->status = STATUS_FAILED;
        } else if (subnode->status == STATUS_CONFIRMED) {
          ast->status = STATUS_ONGOING;
          ast->pos++;
        } else if (subnode->status == STATUS_ONGOING) {
          ast->status = STATUS_ONGOING;
        } 
      }
      if (ast->pos >= ((Array*)bnf->content)->size) {
        ast->status = STATUS_CONFIRMED;
        subnode = at(ast->subnodes, 0);
        for (int i = 1; i < ((Array*)bnf->content)->size; i++) {
          concatastnode(subnode, at(ast->subnodes, i));
        }
        concat(ast->value, newString(subnode->value->content));
      }
      if (ast->status == STATUS_CONFIRMED || ast->status == STATUS_FAILED) {
        while (ast->subnodes->size) {
          freeastnode(pop(ast->subnodes));
        }
      }
      break;
    //////////////////////////////////////////////////////////////////////////
    case NODE_LEAF:
      if (c == AST_LOCK) {
        if (ast->status == STATUS_POTENTIAL) ast->status = STATUS_CONFIRMED;
        else ast->status = STATUS_FAILED;
      } else {
        content = bnf->content;
        if (content[ast->pos] == c) {
          if (!content[ast->pos + 1]) {
            ast->status = bnf->type == con ? STATUS_CONFIRMED : STATUS_POTENTIAL;
          } else {
            ast->status = STATUS_ONGOING;
          }
        } else {
          ast->status = STATUS_FAILED;
        }
        ast->pos++;
      }
      if (ast->status == STATUS_CONFIRMED) {
        concat(ast->value, newString(bnf->content));
      }
      break;
    //////////////////////////////////////////////////////////////////////////
    case NODE_MANY_OR_NONE:
    case NODE_ONE_OR_NONE:
      if (!ast->subnodes->size) {
        if (c == AST_LOCK) {
          ast->status = STATUS_CONFIRMED;
          break;
        }
        subnode = astfrombnf(bnf);
        subnode->status = STATUS_NOSTATUS;
        pushastnode(ast, &subnode);
      } else {
        subnode = at(ast->subnodes, ast->subnodes->size - 1);
      }
      supernode = ast;
      ast = subnode;
    //////////////////////////////////////////////////////////////////////////
    case NODE_LIST:
      if (ast->pos < ((Array*)bnf->content)->size) {
        subnode = at(ast->subnodes, ast->pos);
        subbnf  = at(bnf->content,  ast->pos);
        if (!subnode) {
          subnode = astfrombnf(subbnf);
          subnode->status = STATUS_NOSTATUS;
          pushastnode(ast, &subnode);
        }
        newcharast(reserved, subnode, subbnf, ast->pos == 0 && con, c);
        if (subnode->status == STATUS_FAILED) {
          ast->status = STATUS_FAILED;
          while (ast->subnodes->size) {
            freeastnode(pop(ast->subnodes));
          }
        } else if (subnode->status == STATUS_CONFIRMED || subnode->status == STATUS_PARTIAL) {
          ast->status = STATUS_ONGOING;
          ast->pos++;
        } else if (subnode->status == STATUS_ONGOING) {
          ast->status = STATUS_ONGOING;
        }
      }
      if (ast->pos >= ((Array*)bnf->content)->size) {
        ast->status = STATUS_CONFIRMED;
        if (ast->subnodes->size == 1) {
          ASTNode *node = at(ast->subnodes, 0);
          concat(ast->value, newString(node->value->content));
          concat(ast->name,  newString(node->name->content));
          astswaparray(ast, node);
        }
      }
      //////////////////////////////////////////////////////////////////////
      if (bnf->type == NODE_ONE_OR_NONE || bnf->type == NODE_MANY_OR_NONE) {
        if (ast->status == STATUS_CONFIRMED) {
          if (bnf->type == NODE_MANY_OR_NONE && c != AST_LOCK) {
            ast = astfrombnf(bnf);
            ast->status = STATUS_NOSTATUS;
            pushastnode(supernode, &ast);
          } else {
            supernode->status = STATUS_CONFIRMED;
          }
        } else if (ast->status == STATUS_ONGOING) {
          supernode->status = STATUS_ONGOING;
        } else if (ast->status == STATUS_FAILED) {
          supernode->status = STATUS_FAILED;
          freeastnode(pop(supernode->subnodes));
        }
        if (supernode->status == STATUS_CONFIRMED) {
          if (!ast->subnodes->size) {
            concat(supernode->value, newString(ast->value->content));
            concat(supernode->name,  newString(ast->name->content));
          } 
          astswaparray(supernode, ast); 
        }
      }
      break;
    //////////////////////////////////////////////////////////////////////////
    case NODE_ONE_OF:
      if (c == AST_LOCK && !ast->subnodes->size) {
        ast->status = STATUS_FAILED;
        break;
      }
      for (int i = 0; i < ((Array*)bnf->content)->size; i++) {
        subnode = at(ast->subnodes, ast->pos);
        subbnf  = at(bnf->content,  ast->pos);
        if (!subnode) {
          subnode = astfrombnf(subbnf);
          subnode->status = STATUS_NOSTATUS;
          pushastnode(ast, &subnode);
        }
        if (subnode->status != STATUS_FAILED) {
          newcharast(reserved, subnode, subbnf, con, c);
        }
        if (subnode->status == STATUS_FAILED) {
          ast->status = STATUS_ONGOING;
          ast->pos++;
        } else if (subnode->status == STATUS_CONFIRMED) {
          ast->status = STATUS_CONFIRMED;
          concat(ast->value, newString(subnode->value->content));
          concat(ast->name,  newString(subnode->name->content));
          astswaparray(ast, subnode);
          break;
        } else if (subnode->status == STATUS_ONGOING) {
          ast->status = STATUS_ONGOING;
        }
      }
      if (ast->pos >= ((Array*)bnf->content)->size) {
        ast->status = STATUS_FAILED;
        while (ast->subnodes->size) freeastnode(pop(ast->subnodes));
      }
      break;
  }
  return;
}


ASTNode *parseast(char *filename)
{
  BNFNode      *root     = parsebnf("parsing/bnf/test.bnf");
  Parser       *parser   = newParser("parsing/prs/csr.prs");
  SymbolStream *ss       = ssopen(filename, parser);
  ASTNode      *ast      = astfrombnf(at(root->content, 0));
  Array        *reserved = newArray(sizeof(char*));

  Symbol *s;
  while (!(s = ssgets(ss))->eof) {
    int i = 0;
    char c;
    while ((c = s->text[i++])) {
      newcharast(reserved, ast, at(root->content, 0), 0, c);
    }
    newcharast(reserved, ast, at(root->content, 0), 0, AST_LOCK);
  }

  deleteArray(&reserved);
  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&root);
  return ast;
}

void delasttree(ASTNode *node) {
  for (int i = 0; i < node->subnodes->size; i++) {
      delasttree(at(node->subnodes, i));
    }
    freeastnode(node);
}

void deleteASTTree(ASTNode **node) {
  if (*node) {
    delasttree(*node);
    free(*node);
    *node = NULL;
  }
}