#include <ast.h>

Array *newcharast(ASTNode*, BNFNode*, Array*, char);
void pushastnode(ASTNode*, ASTNode**);

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

ASTNode *newASTNode(ASTNode *ast)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name     = newString(ast->name->content);
    new->subnodes = newArray(sizeof(ASTNode));
    new->value    = newString(ast->value->content);
    new->status   = ast->status;
    new->pos      = ast->pos;
    if (new->subnodes) {
      for (int i = 0; i < ast->subnodes->size; i++) {
        ASTNode *n = newASTNode(at(ast->subnodes, i));
        pushastnode(new, &n);
      }
    }
  }
  return ast;
}

void freeastnode(ASTNode *node) {
  deleteString(&node->name);
  deleteString(&node->value);
  deleteArray(&node->subnodes);
}

void delasttree(ASTNode *node) {
  for (int i = 0; i < node->subnodes->size; i++) {
      delasttree(at(node->subnodes, i));
    }
    freeastnode(node);
}

void concatastnode(ASTNode *a, ASTNode *b) {
  // TODO: ENSURE b->subnodes->size is always 1
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

void upastnode(ASTNode *super, ASTNode *sub) {
      concat(super->value, newString(sub->value->content));
      concat(super->name,  newString(sub->name->content));
      astswaparray(super, sub);
}

void asterror(Array *errors, ASTError error) {
  push(errors, (void*)error);
}

void astnodeleaf(Array *errors, ASTNode *ast, BNFNode *bnf, char c) 
{
  if (c == AST_LOCK) {
    if (ast->status == STATUS_POTENTIAL) {
      ast->status = STATUS_CONFIRMED;
      concat(ast->value, newString(bnf->content));
    }
    else ast->status = STATUS_FAILED;
  } else {
    char *content = bnf->content;
    if (content[ast->pos] == c) {
      if (!content[ast->pos + 1]) {
        ast->status = bnf->type = STATUS_POTENTIAL;
      } else {
        ast->status = STATUS_ONGOING;
      }
    } else {
      ast->status = STATUS_FAILED;
    }
    ast->pos++;
  }
}

void astnodelist(Array *errors, ASTNode *ast, BNFNode *bnf, Array *reserved, char c)
{
  ASTNode *subast;
  ASTNode *superast;
  BNFNode *subbnf;
  int      size  = ((Array*)bnf->content)->size;
  int      split = 0;

  if (!ast->subnodes->size) {
    superast = ast;
    ast = astfrombnf(bnf);
    pushastnode(superast, &ast);
    split = 1;
  } else {
    superast = ast;
  }
  for (int i = 0, lim = superast->subnodes->size; i < lim; i++) {
    ast = at(superast->subnodes, i);
    subast = at(ast->subnodes, ast->pos);
    subbnf = at(bnf->content,  ast->pos);
    if (!subast) {
      subast = astfrombnf(subbnf);
      pushastnode(ast, &subast);
    }
    /////////////////////////////////////////////////////////////
    if (split                            &&
       (subbnf->type == NODE_ONE_OR_NONE ||
        subbnf->type == NODE_MANY_OR_NONE))
    {
      ASTNode *n = newASTNode(ast);
      pushastnode(superast, &n);
      n->pos++;
      lim++;
    }
    combine(errors, newcharast(subast, subbnf, reserved, c));
    if (subast->status == STATUS_PARTIAL) {
      ASTNode *n = newASTNode(ast);
      pushastnode(superast, &n);
      newcharast(subast, subbnf, reserved, AST_CLOSE);
      if (ast->pos == size - 1) ast->pos--;
    }
    if (subast->status == STATUS_FAILED) {
      ast->status = STATUS_FAILED;
      while (ast->subnodes->size) freeastnode(pop(ast->subnodes));
      freeastnode(rem(superast->subnodes, i--));
    } else if (subast->status == STATUS_CONFIRMED) {
      ast->status = STATUS_ONGOING;
      ast->pos++;
    } else if (subast->status == STATUS_ONGOING) {
      ast->status = STATUS_ONGOING;
    } else if (subast->status == STATUS_POTENTIAL) {
      if (ast->pos == size - 1) ast->status = STATUS_POTENTIAL;
      else ast->status = STATUS_ONGOING;
    }
    if (!superast->subnodes->size) {
      superast->status = STATUS_FAILED;
      break;
    }
    if (ast->status == STATUS_POTENTIAL) {
      superast->status = STATUS_POTENTIAL;
    }
    if (ast->pos >= size) {
      ast->status = STATUS_CONFIRMED;
      superast->status = STATUS_CONFIRMED;
      if (ast->subnodes->size == 1) {
        ASTNode *node = at(ast->subnodes, 0);
        upastnode(ast, node);
      }
      upastnode(superast, ast);
      // delete
      break;
    }
  }
}

void astnodeconcat(Array *errors, ASTNode *ast, BNFNode *bnf, Array *reserved, char c)
{
  ASTNode *superast;
  ASTNode *subast;
  BNFNode *subbnf;

  if (!ast->subnodes->size) {
    superast = ast;
    ast = astfrombnf(bnf);
    pushastnode(superast, &ast);
  } else {
    superast = ast;
  }
  for (int i = 0, lim = superast->subnodes->size; i < lim; i++) {
    ast = at(superast->subnodes, i);
    if (ast->pos < 2) {
      subast = at(ast->subnodes, ast->pos);
      subbnf = at(bnf->content,  ast->pos);
      if (!subast) {
        subast = astfrombnf(subbnf);
        pushastnode(ast, &subast);
      }
      combine(errors, newcharast(subast, subbnf, reserved, c));
      if (subast->status == STATUS_POTENTIAL) {
        ast->status = STATUS_ONGOING;
        if (superast->status == STATUS_NOSTATUS) superast->status = STATUS_ONGOING;
        if (!ast->pos) {
          ASTNode *n = newASTNode(ast);
          pushastnode(superast, &n);
          ast->pos++;
          newcharast(subast, subbnf, reserved, AST_LOCK);
        }
      } else if (subast->status == STATUS_ONGOING) {
        ast->status = STATUS_ONGOING;
        if (superast->status == STATUS_NOSTATUS) superast->status = STATUS_ONGOING;
      } else if ((subast->status == STATUS_CONFIRMED) && 
                  (ast->pos == ((Array*)bnf->content)->size - 1))
      {
        ast->status = STATUS_CONFIRMED;
      } else {
        ast->status = STATUS_FAILED;
        delasttree(rem(superast->subnodes, i));
        i--;
      }
    }
  }
  if (c == AST_LOCK) {
    if (superast->subnodes->size != 1) {
      superast->status = STATUS_FAILED;
      if (!superast->subnodes->size) {
        asterror(errors, ERROR_CONCAT_0MATCH);
      } else {
        asterror(errors, ERROR_CONCAT_MANYMATCH);
      }
    } else {
      superast->status = STATUS_CONFIRMED;
      ast = at(superast->subnodes, 0);
      concat(superast->value, newString(ast->value->content));
    }
    while (superast->subnodes->size) {
      freeastnode(pop(superast->subnodes));
    }
  }
}

void astnodeoneof(Array *errors, ASTNode *ast, BNFNode *bnf, Array *reserved, char c)
{
  ASTNode *superast;
  ASTNode *subast;
  BNFNode *subbnf;
  int      size    = ((Array*)bnf->content)->size;
  int      split   = 0;

  if (!ast->subnodes->size) {
    superast = ast;
    ast = astfrombnf(bnf);
    pushastnode(superast, &ast);
    split = 1;
  } else {
    superast = ast;
    ast = at(superast->subnodes, 0);
  }
  for (int i = 0; i < size; i++) {
    subast = at(ast->subnodes, i);
    subbnf = at(bnf->content,  i);
    if (!subast) {
      subast = astfrombnf(subbnf);
      pushastnode(ast, &subast);
    }
    ///////////////////////////////////////////////////////////
    if (split                            &&
       (subbnf->type == NODE_ONE_OR_NONE ||
        subbnf->type == NODE_MANY_OR_NONE))
    {
      ASTNode *n = newASTNode(ast);
      n->status = STATUS_PARTIAL;
      pushastnode(superast, &n);
      split = 0;
    }
    combine(errors, newcharast(subast, subbnf, reserved, c));
    if (subast->status == STATUS_PARTIAL) {
      ASTNode *n = newASTNode(ast);
      n->status = STATUS_PARTIAL;
      if (superast->subnodes->size > 1) {
        freeastnode(at(superast->subnodes, 2));
        set(superast->subnodes, 2, n);
        free(n);
      } else {
        pushastnode(superast, &n);
      }
    }
    if (subast->status == STATUS_CONFIRMED) {
      ast->status = STATUS_CONFIRMED;
      upastnode(ast, subast);
      upastnode(superast, ast);
      break;
    } else if (subast->status == STATUS_POTENTIAL) {
      ast->status = STATUS_POTENTIAL;
    } else if (subast->status == STATUS_FAILED) {
      if (ast->status == STATUS_NOSTATUS) ast->status = STATUS_ONGOING;
      ast->pos++;
    } else if (ast->status == STATUS_NOSTATUS) {
      ast->status = STATUS_ONGOING;
    }
  }
  if (ast->pos >= size) {
    superast->status = STATUS_FAILED;
    while (ast->subnodes->size)      freeastnode(pop(ast->subnodes));
    while (superast->subnodes->size) freeastnode(pop(superast->subnodes));
  }
}

void astnodemanyornone(Array *errors, ASTNode *ast, BNFNode *bnf, Array *reserved, char c)
{
  //ASTNode *superast;
  //ASTNode *subast;
  //BNFNode *subbnf;

  
}

void astnodeoneornone(Array *errors, ASTNode *ast, BNFNode *bnf, Array *reserved, char c)
{

}

Array *newcharast(ASTNode *ast, BNFNode *bnf, Array *reserved, char c) {
  Array   *errors = newArray(sizeof(ASTError));

  switch (bnf->type) {
    case NODE_ROOT:
      break;
    case NODE_LEAF:
      astnodeleaf(errors, ast, bnf, c);
      break;
    case NODE_CONCAT:
      astnodeconcat(errors, ast, bnf, reserved, c);
      break;
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
      astnodemanyornone(errors, ast, bnf, reserved, c);
      break;
    case NODE_ONE_OR_NONE:
      astnodeoneornone(errors, ast, bnf, reserved, c);
      break;
    case NODE_LIST:
      astnodelist(errors, ast, bnf, reserved, c);
      break;
    case NODE_ONE_OF:
      astnodeoneof(errors, ast, bnf, reserved, c);
      break;
  }

  return errors;
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
      newcharast(ast, at(root->content, 0), reserved, c);
    }
    //newcharast(reserved, ast, at(root->content, 0), 0, AST_LOCK);
  }

  deleteArray(&reserved);
  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&root);
  return ast;
}

void deleteASTTree(ASTNode **node) {
  if (*node) {
    delasttree(*node);
    free(*node);
    *node = NULL;
  }
}