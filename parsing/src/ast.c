#include <ast.h>

void pushastnode(ASTNode *node, ASTNode **ref) {
  push(node->subnodes, *ref);
  free(*ref);
  *ref = at(node->subnodes, node->subnodes->size - 1);
}

ASTNode *astfrombnf(BNFNode *bnf)
{
  ASTNode *ast = malloc(sizeof(ASTNode));
  if (ast) {
    ast->name     = newString(bnf->name);
    ast->subnodes = newArray(sizeof(ASTNode));
    ast->value    = newString("");
    ast->status   = STATUS_NOSTATUS;
    ast->ref      = bnf;
    ast->pos      = 0;
  }
  return ast;
}

void freeastnode(ASTNode *node) {
  deleteString(&node->name);
  deleteString(&node->value);
  deleteArray(&node->subnodes);
}

void newcharast(ASTNode *ast, BNFNode *bnf, int con, char c)
{
  ASTNode *subnode;
  ASTNode *supernode;
  ASTNode *trickle;
  int fail, success;
  char *content;
  switch (bnf->type) {
    case NODE_ROOT:
      break;
    //////////////////////////////////////////////////////////////////////////
    case NODE_CONCAT:
      if (c == AST_LOCK) {
        if (!ast->subnodes->size) {

        }
      } else {
        for (int i = 0; i < 2; i++) {
          subnode = at(ast->subnodes, i);
          if (!subnode) {
            subnode = astfrombnf(bnf);
            subnode->status = STATUS_NOSTATUS;
            pushastnode(ast, &subnode);
          }
          if (subnode->status != STATUS_CONFIRMED && subnode->status != STATUS_FAILED) {
            i = 2;
          }
          if (subnode->status == STATUS_ONGOING) {
            ast->status = STATUS_ONGOING;
          }
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
    case NODE_ONE_OR_NONE:
    case NODE_MANY_OR_NONE:
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
      success = 0;
      for (int i = 0; i < ((Array*)bnf->content)->size; i++) {
        subnode = at(ast->subnodes, i);
        if (!subnode) {
          subnode = astfrombnf(at(bnf->content, i));
          subnode->status = STATUS_NOSTATUS;
          pushastnode(ast, &subnode);
        }
        if (subnode->status != STATUS_CONFIRMED) {
          newcharast(subnode, at(bnf->content, i), i == 0 && con, c);
          // Break the loop at next iteration
          i = ((Array*)bnf->content)->size;
        } 
        if (subnode->status == STATUS_FAILED) {
          ast->status = STATUS_FAILED;
          break;
        } else if (subnode->status == STATUS_CONFIRMED) {
          success++;
        } else if (subnode->status == STATUS_ONGOING) {
          ast->status = STATUS_ONGOING;
        }
      }
      if (success == ((Array*)bnf->content)->size) {
        ast->status = STATUS_CONFIRMED;
      }
      if (ast->status == STATUS_FAILED) {
        for (int i = 0; i < ast->subnodes->size; i++) {
          ASTNode *del = at(ast->subnodes, i);
          deleteASTTree(&del);  
        }
      }
      if (ast->status == STATUS_CONFIRMED && ast->subnodes->size == 1) {
        trickle = pop(ast->subnodes);
        concat(ast->value, newString(trickle->value->content));
        concat(ast->name,  newString(trickle->name->content));
        freeastnode(trickle);
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
        }
        if (supernode->status == STATUS_CONFIRMED) {
          if (!ast->subnodes->size) {
            concat(supernode->value, newString(ast->value->content));
            concat(supernode->name,  newString(ast->name->content));
            freeastnode(ast);
          } else if (ast->subnodes->size == 1) {
            Array *tmp = supernode->subnodes;
            supernode->subnodes = ast->subnodes;
            ast->subnodes = tmp;
            freeastnode(ast);
          }
        }
      }
      break;
    //////////////////////////////////////////////////////////////////////////
    case NODE_ONE_OF:
      if (c == AST_LOCK && !ast->subnodes->size) {
        ast->status = STATUS_FAILED;
        break;
      }
      fail = 0;
      success = 0;
      trickle = NULL;
      for (int i = 0; i < ((Array*)bnf->content)->size; i++) {
        subnode = at(ast->subnodes, i);
        if (!subnode) {
          subnode = astfrombnf(at(bnf->content, i));
          subnode->status = STATUS_NOSTATUS;
          pushastnode(ast, &subnode);
        }
        if (subnode->status != STATUS_FAILED) {
          newcharast(subnode, at(bnf->content, i), con, c);
        }
        if (subnode->status == STATUS_FAILED) {
          fail++;
        } else if (subnode->status == STATUS_CONFIRMED) {
          ast->status = STATUS_CONFIRMED;
          success++;
          trickle = subnode;
        } else if (subnode->status == STATUS_ONGOING && !success) {
          ast->status = STATUS_ONGOING;
        }
      }
      if (success > 1) {
        // priority
      }
      if (fail == ast->subnodes->size) {
        ast->status = STATUS_FAILED;
      }
      if (c == AST_LOCK && ast->status != STATUS_CONFIRMED) {
        ast->status = STATUS_FAILED;
      }
      if (ast->status == STATUS_CONFIRMED || ast->status == STATUS_FAILED) {
        for (int i = 0; i < ast->subnodes->size; i++) {
          ASTNode *del = rem(ast->subnodes, i);
          if (del != trickle) {
            freeastnode(del);
          }
        }
        if (trickle) {
          concat(ast->value, newString(trickle->value->content));
          concat(ast->name,  newString(trickle->name->content));
          freeastnode(pop(ast->subnodes));
        }
      }
      break;
  }
  return;
}


ASTNode *parseast(char *filename)
{
  BNFNode      *root   = parsebnf("parsing/bnf/test.bnf");
  Parser       *parser = newParser("parsing/prs/csr.prs");
  SymbolStream *ss     = ssopen(filename, parser);
  //ASTNode      *ast    = initast(root);

  Symbol *s;
  while (!(s = ssgets(ss))->eof) {
    printf("%s\n", s->text);
  }

  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&root);
  return NULL;
}

void deleteASTTree(ASTNode **node) {
  if (*node) {
    for (int i = 0; i < (*node)->subnodes->size; i++) {
      ASTNode *subnode = at((*node)->subnodes, i);
      deleteASTTree(&subnode);
    }
    freeastnode(*node);
    free(*node);
    *node = NULL;
  }
}