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

void concatastnode(ASTNode *a, ASTNode *b) {
  for (int i = 0; i < b->subnodes->size; i++) {
    concatastnode(b, at(b->subnodes, i));
  }
  concat(a->name,  newString(b->name->content));
  concat(a->value, newString(b->value->content));
}

void newcharast(Array *reserved, ASTNode *ast, BNFNode *bnf, int con, char c)
{
  ASTNode *subnode;
  ASTNode *supernode;
  BNFNode *subbnf;
  char    *content;

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
        newcharast(reserved, subnode, subbnf, 1, c);
        if (subnode->status == STATUS_FAILED) {
          ast->status = STATUS_FAILED;
        } else if (subnode->status == STATUS_CONFIRMED) {
          ast->pos++;
        } else if (subnode->status == STATUS_ONGOING) {
          ast->status = STATUS_ONGOING;
        } 
      }
      if (ast->status == STATUS_CONFIRMED) {
        subnode = at(ast->subnodes, 0);
        concatastnode(subnode, at(ast->subnodes, 1));
        concat(ast->value, newString(subnode->value->content));
        concat(ast->name,  newString(subnode->name->content));
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
        } else if (subnode->status == STATUS_CONFIRMED) {
          ast->pos++;
        } else if (subnode->status == STATUS_ONGOING) {
          ast->status = STATUS_ONGOING;
        }
      }
      if (ast->pos >= ((Array*)bnf->content)->size) {
        ast->status = STATUS_CONFIRMED;
        if (ast->subnodes->size == 1) {
          ASTNode *node = pop(ast->subnodes);
          concat(ast->value, newString(node->value->content));
          concat(ast->name,  newString(node->name->content));
          freeastnode(node);
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
      if (ast->pos < ((Array*)bnf->content)->size) {
        subnode = at(ast->subnodes, ast->pos);
        subbnf  = at(bnf->content,  ast->pos);
        if (!subnode) {
          subnode = astfrombnf(subbnf);
          subnode->status = STATUS_NOSTATUS;
          pushastnode(ast, &subnode);
        }
        newcharast(reserved, subnode, subbnf, con, c);
        if (subnode->status == STATUS_FAILED) {
          ast->pos++;
        } else if (subnode->status == STATUS_CONFIRMED) {
          ast->status = STATUS_CONFIRMED;
          concat(ast->value, newString(subnode->value->content));
          concat(ast->name,  newString(subnode->name->content));
        } else if (subnode->status == STATUS_ONGOING) {
          ast->status = STATUS_ONGOING;
        }
      }
      if (ast->pos >= ((Array*)bnf->content)->size) {
        ast->status = STATUS_FAILED;
      }
      if (ast->status == STATUS_CONFIRMED || ast->status == STATUS_FAILED) {
        while (ast->subnodes->size) {
          freeastnode(pop(ast->subnodes));
        }
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

  // Symbol *s;
  // while (!(s = ssgets(ss))->eof) {
  //   printf("%s\n", s->text);
  // }

  newcharast(reserved, ast, at(root->content, 0), 0, '0');
  newcharast(reserved, ast, at(root->content, 0), 0, '1');
  newcharast(reserved, ast, at(root->content, 0), 0, '0');
  newcharast(reserved, ast, at(root->content, 0), 0, AST_LOCK);

  deleteArray(&reserved);
  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&root);
  return ast;
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