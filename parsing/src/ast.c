#include <ast.h>

Array *newcharast(ASTNode*, BNFNode*, Array*, char);
ASTNode *astsubnode(ASTNode*, int);

ASTNode *astfrombnf(BNFNode *bnf)
{
  ASTNode *ast = malloc(sizeof(ASTNode));
  if (ast) {
    ast->name     = newString(bnf->name);
    ast->subnodes = newArray(sizeof(ASTNode*));
    ast->value    = newString("");
    ast->status   = STATUS_NOSTATUS;
    ast->pos      = 0;
  }
  return ast;
}

ASTNode *newASTNode(ASTNode *ast)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name     = newString(ast->name->content);
    new->subnodes = newArray(sizeof(ASTNode*));
    new->value    = newString(ast->value->content);
    new->status   = ast->status;
    new->pos      = ast->pos;
    if (new->subnodes) {
      for (int i = 0; i < ast->subnodes->size; i++) {
        ASTNode *n = newASTNode(astsubnode(ast, i));
        push(new->subnodes, &n);
      }
    }
  }
  return new;
}

ASTNode *astsubnode(ASTNode *ast, int index) {
  ASTNode **ret = (ASTNode**)at(ast->subnodes, index);
  if (ret) return *ret;
  return NULL;
}

BNFNode *bnfsubnode(BNFNode *bnf, int index) {
  return *(BNFNode**)at(bnf->content, index);
}

void freeastnode(ASTNode *node) {
  deleteString(&node->name);
  deleteString(&node->value);
  deleteArray(&node->subnodes);
}

void concatastnode(ASTNode *c) {
  // TODO: ENSURE b->subnodes->size is always 2
  for (int i = 0; i < c->subnodes->size; i++) {
    concatastnode(astsubnode(c, i));
  }
  for (int i = 0; i < c->subnodes->size; i++) {
    ASTNode *n = *(ASTNode**)at(c->subnodes, i);
    concat(c->value, newString(n->value->content));
    deleteAST(&n);
  }
  // not legit
  c->subnodes->size = 0;
}

void astswaparray(ASTNode *super, ASTNode *sub) {
  Array *tmp = sub->subnodes;
  sub->subnodes = newArray(sizeof(ASTNode*));
  while (super->subnodes->size) deleteAST(pop(super->subnodes));
  deleteArray(&super->subnodes);
  super->subnodes = tmp;
}

void upastnode(ASTNode *super, ASTNode *sub) {
  concat(super->value, newString(sub->value->content));
  if (!super->name->content[0]) {
    concat(super->name,  newString(sub->name->content));
  }
  astswaparray(super, sub);
}

int astkeep(ASTNode *super, int partial) {
  int ret = 1;
  ASTNode *n, *keep = NULL;
  for (int i = 0; super->subnodes->size; ) {
    n = *(ASTNode**)pop(super->subnodes);
    if ((!partial || n->status != STATUS_PARTIAL) && n->status != STATUS_CONFIRMED) {
      deleteAST(&n);
    } else {
      if (keep) deleteAST(&keep);
      keep = n;
      i++;
    };
    if (i == 2) {
      // AMBIGUOUS BNF TREE
      ret = 0;
      break;
    }
  }
  upastnode(super, keep);
  return ret;
}

void asterror(Array *errors, ASTError error) {
  push(errors, &error);
}

void astcheckrecnode(ASTNode *node) {
  String   *rec = newString(REC_NODE_INDICATOR);
  ASTNode **l   = last(node->subnodes);
  if (l) {
    if (contains((*l)->name, rec)) {
      if ((*l)->subnodes->size) {
        ASTNode *n = *(ASTNode**)pop(node->subnodes);
        while (n->subnodes->size) push(node->subnodes, rem(n->subnodes, 0));
        deleteAST(&n);
      } else {
        deleteAST(pop(node->subnodes));
      }
    }
  }
}

Array *newcharast(ASTNode *ast, BNFNode *bnf, Array *reserved, char c) {
  Array   *errors = newArray(sizeof(ASTError));
  ASTNode *superast;
  ASTNode *subast;
  BNFNode *subbnf;
  int      size = 0;
  if (bnf->type != NODE_LEAF) size = ((Array*)bnf->content)->size;

  switch (bnf->type) {
    case NODE_ROOT:
      break;
    case NODE_LEAF:
      if (c == AST_LOCK || c == AST_CLOSE) {
        if (bnf->content && (ast->pos || c == AST_CLOSE)) ast->status = STATUS_FAILED;
        else if (bnf->content)                            ast->status = STATUS_ONGOING;
        else                                              ast->status = STATUS_CONFIRMED;
      } else if (c == AST_CONCAT) {
        if (bnf->content) ast->status = STATUS_ONGOING;
        else              ast->status = STATUS_CONFIRMED;
      } else {
        char *content = bnf->content;
        if (content && content[ast->pos] == c) {
          if (!content[ast->pos + 1]) {
            ast->status = STATUS_CONFIRMED;
          } else {
            ast->status = STATUS_ONGOING;
          }
        } else {
          ast->status = STATUS_FAILED;
        }
        ast->pos++;
      }
      if (ast->status == STATUS_CONFIRMED && bnf->content) {
        concat(ast->value, newString(bnf->content));
      }
      break;
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
    case NODE_ONE_OR_NONE:
      /// UNIMPLEMENTED
      asterror(errors, ERROR_UNIMPLEMENTED);
      break;
    case NODE_CONCAT:
    case NODE_LIST:
      superast = ast;
      if (!ast->subnodes->size) {
        ast = astfrombnf(bnf);
        push(superast->subnodes, &ast);
      }
      for (int i = 0, lim = superast->subnodes->size; i < lim; i++) {
        ast = astsubnode(superast, i);
        if (ast->status == STATUS_FAILED) continue;
        //////////////////////////////////////////////////
        subast = astsubnode(ast, ast->pos);
        subbnf = bnfsubnode(bnf, ast->pos);
        if (!subast) {
          subast = astfrombnf(subbnf);
          push(ast->subnodes, &subast);
        }
        combine(errors, newcharast(subast, subbnf, reserved, c));
        if (subast->status == STATUS_FAILED) {
          superast->status = STATUS_ONGOING;
          deleteAST(rem(superast->subnodes, i--));
          lim--;
          continue;
        } else if (subast->status == STATUS_PARTIAL) {
          ast->status = STATUS_PARTIAL;
          superast->status = STATUS_ONGOING;
          ASTNode *n;
          if (ast->pos + 1 < size) {
            n = newASTNode(ast);
            push(superast->subnodes, &n);
            newcharast(subast, subbnf, reserved, AST_CLOSE);
          }
          ++ast->pos;
        } else if (subast->status == STATUS_CONFIRMED) {
          superast->status = STATUS_ONGOING;
          ++ast->pos;
          if (c == AST_CLOSE) i--;
        } else if (superast->status == STATUS_NOSTATUS) {
          superast->status = STATUS_ONGOING;
          ast->status = STATUS_ONGOING;
        }
        if (ast->pos >= size) {
          superast->status = STATUS_CONFIRMED;
          while (superast->subnodes->size) {
            ASTNode *n = *(ASTNode**)pop(superast->subnodes);
            if (n != ast) deleteAST(&n);
          }
          push(superast->subnodes, &ast);
          ast->status = STATUS_CONFIRMED;
          if (ast->subnodes->size == 1) {
            upastnode(ast, subast);
          }
          if (superast->subnodes->size == 1) {
            upastnode(superast, ast);
          }
          astcheckrecnode(superast);
          break;
        }
      }
      if (!superast->subnodes->size) {
        superast->status = STATUS_FAILED;
      }
      if ((c == AST_LOCK || c == AST_CLOSE) && bnf->type == NODE_CONCAT) {
        concatastnode(superast);
        // CHECK IF CONCAT IS RESERVED
      }
      break;
    case NODE_ONE_OF:
      for (int i = 0; i < size; i++) {
        subast = astsubnode(ast, i);
        subbnf = bnfsubnode(bnf, i);
        if (!subast) {
          subast = astfrombnf(subbnf);
          push(ast->subnodes, &subast);
        }
        if (subast->status != STATUS_FAILED) {
          combine(errors, newcharast(subast, subbnf, reserved, c));
          if (subast->status == STATUS_CONFIRMED || subast->status == STATUS_PARTIAL) {
            if (c == AST_CLOSE) ast->status = STATUS_CONFIRMED;
            else                ast->status = STATUS_PARTIAL;
          } else if (subast->status == STATUS_FAILED) {
            ast->pos++;
          } else if (ast->status == STATUS_NOSTATUS) {
            ast->status = STATUS_ONGOING;
          }
        }
      }
      if ((ast->status == STATUS_PARTIAL && ast->pos == size - 1) || ast->status == STATUS_CONFIRMED) {
        ast->status = STATUS_CONFIRMED;
        astkeep(ast, 1);
      } else if (ast->pos >= size) {
        ast->status = STATUS_FAILED;
        while(ast->subnodes->size) deleteAST(pop(ast->subnodes));
      }
      break;
  }

  return errors;
}

ASTNode *parseast(char *filename)
{
  BNFNode      *bnftree  = parsebnf("parsing/bnf/test.bnf");
  BNFNode      *rootent  = bnfsubnode(bnftree, 0);
  Parser       *parser   = newParser("parsing/prs/csr.prs");
  SymbolStream *ss       = ssopen(filename, parser);
  ASTNode      *ast      = astfrombnf(rootent);
  Array        *reserved = newArray(sizeof(char*));

  Symbol *s;
  while (!(s = ssgets(ss))->eof) {
    int i = 0;
    char c;

    newcharast(ast, rootent, reserved, AST_LOCK);
    while ((c = s->text[i++])) {
      newcharast(ast, rootent, reserved, c);
    }
  }
  newcharast(ast, rootent, reserved, AST_CLOSE);

  deleteArray(&reserved);
  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&bnftree);
  return ast;
}

void deleteAST(ASTNode **node) {
  if (*node) {
    while((*node)->subnodes->size) deleteAST(pop((*node)->subnodes));
    freeastnode(*node);
    free(*node);
    *node = NULL;
  }
}