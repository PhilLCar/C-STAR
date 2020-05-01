#include <ast.h>

void astnewchar(Array*, ASTNode*, BNFNode*, Array*, int, char);
ASTNode *astsubnode(ASTNode*, int);

ASTNode *newASTNode(ASTNode *ast, BNFNode *bnf)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name     = newString(bnf->name);
    new->parent   = ast;
    new->ref      = bnf;
    new->subnodes = newArray(sizeof(ASTNode*));
    new->value    = newString("");
    new->status   = STATUS_NOSTATUS;
    new->pos      = 0;
    new->partial  = 0;
    if (ast) push(ast->subnodes, &new);
  }
  return new;
}

ASTNode *duplicateAST(ASTNode *ast)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name     = newString(ast->name->content);
    new->parent   = ast->parent;
    new->ref      = ast->ref;
    new->subnodes = newArray(sizeof(ASTNode*));
    new->value    = newString(ast->value->content);
    new->status   = ast->status;
    new->pos      = ast->pos;
    new->partial  = ast->partial;
    if (new->subnodes) {
      for (int i = 0; i < ast->subnodes->size; i++) {
        ASTNode *n = duplicateAST(astsubnode(ast, i));
        n->parent = new;
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
  deleteAST(&sub); // potential double free
  super->subnodes = tmp;
}

void upastnode(ASTNode *super, ASTNode *sub) {
  concat(super->value, newString(sub->value->content));
  if (!super->name->content[0]) {
    concat(super->name,  newString(sub->name->content));
  }
  astswaparray(super, sub);
}

void asterror(Array *errors, ASTErrorType errno, BNFNode *bnf) {
  ASTError error;
  error.errno  = errno;
  error.bnfref = bnf;
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

void astnewchar(Array *errors, ASTNode *ast, BNFNode *bnf, Array *reserved, int concatmode, char c) {
  ASTNode *superast;
  ASTNode *subast;
  BNFNode *subbnf;
  int      size    = 0;
  char    *content = bnf->content;
  if (bnf->type != NODE_LEAF) size = ((Array*)bnf->content)->size;

  switch (bnf->type) {
    case NODE_ROOT:
      break;
    case NODE_LEAF:
      if (c == AST_LOCK) {
        if (ast->status == STATUS_CONFIRMED) {                                 break; } 
        else if (!content)                   { ast->status = STATUS_PARTIAL;   break; }
        else if (!ast->pos)                  { ast->status = STATUS_ONGOING;   break; }
      } else if (c == AST_CLOSE) {
        if (ast->status == STATUS_CONFIRMED) {                                 break; } 
        else if (!content)                   { ast->status = STATUS_CONFIRMED; break; }
        else if (!ast->pos)                  { ast->status = STATUS_FAILED;    break; }
        c = 0;
      } else if (ast->status == STATUS_CONFIRMED) ast->status = STATUS_FAILED;
      if (content && ast->status != STATUS_FAILED && content[ast->pos] == c) {
        if (!c) {
          ast->status = STATUS_CONFIRMED;
          concat(ast->value, newString(bnf->content));
        } else if (concatmode && !content[ast->pos + 1]) {
          ast->status = STATUS_PARTIAL;
        } else {
          ast->status = STATUS_ONGOING;
        }
        ast->pos++;
      } else {
        ast->status = STATUS_FAILED;
      }
      break;
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
    case NODE_ONE_OR_NONE:
      /// UNIMPLEMENTED
      asterror(errors, ERROR_UNIMPLEMENTED, bnf);
      break;
    case NODE_CONCAT:
    case NODE_LIST:
      superast = ast;
      if (!ast->subnodes->size) ast = newASTNode(superast, bnf);
      superast->status = STATUS_ONGOING;
      for (int i = 0, lim = superast->subnodes->size; i < lim; i++) {
        ast = astsubnode(superast, i);
        ast->status = STATUS_ONGOING;
        //////////////////////////////////////////////////
        subast = astsubnode(ast, ast->pos);
        subbnf = bnfsubnode(bnf, ast->pos);
        if (!subast) subast = newASTNode(ast, subbnf);
        for (char nc = c; ast->pos < size; ) {
          astnewchar(errors,
                     subast,
                     subbnf,
                     reserved, 
                     concatmode || bnf->type == NODE_CONCAT,
                     nc);
          if (subast->status == STATUS_FAILED) {
            deleteAST(rem(superast->subnodes, i--)); lim--;
          } else if (subast->status == STATUS_PARTIAL) {
            if (subbnf->type == NODE_LEAF) {
              nc = AST_CLOSE;
              continue;
            } else {
              ASTNode *n = newASTNode(superast, bnf);
              for (int j = 0; j < ast->pos; j++) {
                ASTNode *completed = duplicateAST(astsubnode(ast, j));
                push(n->subnodes, &completed);
              }
              push(n->subnodes, pop(ast->subnodes));
              for (int j = 0; j < subast->subnodes->size; j++) {
                ASTNode *partial = astsubnode(subast, j);
                if (partial->status == STATUS_PARTIAL) {
                  push(ast->subnodes, rem(subast->subnodes, j));
                }
              }
            }



              ASTNode *n = duplicateAST(ast);
              push(superast->subnodes, &n);
              nc = AST_CLOSE;
              ast->partial = 1;
              continue;
            // } else {
            //   ast->partial = 1;
            //   //ast->pos++;
            // }
          } else if (subast->status == STATUS_CONFIRMED) {
            ++ast->pos;
            if (c == AST_CLOSE || c == AST_LOCK) i--;
          }
          break;
        } 
        if (ast->pos >= size) {
          superast->status = ast->partial ? STATUS_PARTIAL : STATUS_CONFIRMED;
          // while (superast->subnodes->size) {
          //   ASTNode *n = *(ASTNode**)pop(superast->subnodes);
          //   if (n != ast) deleteAST(&n);
          // }
          // push(superast->subnodes, &ast);
          if (ast->subnodes->size == 1) {
            //upastnode(ast, subast);
          }
          if (superast->subnodes->size == 1) {
            //upastnode(superast, ast);
          }
          //astcheckrecnode(superast); //MEMORY LEAK
        }
      }
      // TMP
      // if (superast->subnodes->size) {
      //   for (int i = 0; i < superast->subnodes->size; i++) {
      //     if (astsubnode(superast, i)->status == STATUS_FAILED) count++;
      //   }
      //   if (count == superast->subnodes->size) superast->status = STATUS_FAILED;
      // }
      if (!superast->subnodes->size) {
        superast->status = STATUS_FAILED;
      }
      if ((c == AST_LOCK || c == AST_CLOSE) && bnf->type == NODE_CONCAT) {
        //concatastnode(superast);
        // CHECK IF CONCAT IS RESERVED
      }
      break;
    case NODE_ONE_OF:
      superast = ast;
      if (!ast->subnodes->size) ast = newASTNode(superast, bnf);
      superast->status = STATUS_ONGOING;
      for (int j = 0, lim = superast->subnodes->size; j < lim; j++) {
        ast = astsubnode(superast, j);
        ast->status = STATUS_ONGOING;
        //////////////////////////////////////////////////
        for (int i = 0; i < size; i++) {
          subast = astsubnode(ast, i);
          subbnf = bnfsubnode(bnf, i);
          if (!subast) subast = newASTNode(ast, subbnf);
          if (subast->status != STATUS_FAILED) {
            astnewchar(errors, subast, subbnf, reserved, concatmode, c);
            if (subast->status == STATUS_CONFIRMED)  { ast->status = STATUS_CONFIRMED; break; } 
            else if (subast->status == STATUS_PARTIAL) ast->status = STATUS_PARTIAL;
            else if (subast->status == STATUS_FAILED)  ast->pos++;
          }
        }
        if (ast->status == STATUS_CONFIRMED) {
          superast->status = STATUS_CONFIRMED;
          while (ast->subnodes->size) {
            ASTNode *n = *(ASTNode**)pop(ast->subnodes);
            if (n != subast) deleteAST(&n);
          }
          upastnode(ast, subast);
          //upastnode(superast, ast);
          break;
        } else if (ast->status == STATUS_PARTIAL) {
          superast->status = STATUS_PARTIAL;
          ASTNode *n = duplicateAST(ast);
          n->status = STATUS_ONGOING;
          push(superast->subnodes, &n);
        } else  if (ast->pos >= size) {
          deleteAST(rem(superast->subnodes, j--)); lim--;
        }
      }
      if (!superast->subnodes->size) {
        superast->status = STATUS_FAILED;
      }
      break;
  }
}

ASTNode *parseast(char *filename)
{
  BNFNode      *bnftree  = parsebnf("parsing/bnf/test.bnf");
  BNFNode      *rootent  = bnfsubnode(bnftree, 0);
  Parser       *parser   = newParser("parsing/prs/csr.prs");
  SymbolStream *ss       = ssopen(filename, parser);
  ASTNode      *ast      = newASTNode(NULL, rootent);
  Array        *reserved = newArray(sizeof(char*));
  Array        *errors   = newArray(sizeof(ASTError));

  Symbol *s;
  while (!(s = ssgets(ss))->eof) {
    int i = 0;
    char c;

    astnewchar(errors, ast, rootent, reserved, 0, AST_LOCK);
    while ((c = s->text[i++])) {
      astnewchar(errors, ast, rootent, reserved, 0, c);
    }
  }
  //combine(errors, newcharast(ast, rootent, reserved, 0, AST_CLOSE));
  //combine(errors, newcharast(ast, rootent, reserved, 0, AST_LOCK));

  deleteArray(&errors);
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