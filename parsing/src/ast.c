#include <ast.h>

void astnewchar(Array*, ASTNode*, BNFNode*, Array*, int, char);
ASTNode *astsubnode(ASTNode*, int);

ASTNode *newASTNode(ASTNode *ast, BNFNode *bnf)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name     = newString(bnf->name);
    new->ref      = bnf;
    new->subnodes = newArray(sizeof(ASTNode*));
    new->value    = newString("");
    new->status   = STATUS_NOSTATUS;
    new->pos      = 0;
    if (ast) push(ast->subnodes, &new);
  }
  return new;
}

ASTNode *duplicateAST(ASTNode *ast)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name     = newString(ast->name->content);
    new->ref      = ast->ref;
    new->subnodes = newArray(sizeof(ASTNode*));
    new->value    = newString(ast->value->content);
    new->status   = ast->status;
    new->pos      = ast->pos;
    if (new->subnodes) {
      for (int i = 0; i < ast->subnodes->size; i++) {
        ASTNode *n = duplicateAST(astsubnode(ast, i));
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

void astconcatnode(ASTNode *c) {
  /// TODO: Only add child of type CONCAT
  for (int i = 0; i < c->subnodes->size; i++) {
    astconcatnode(astsubnode(c, i));
  }
  for (int i = 0; i < c->subnodes->size; i++) {
    ASTNode *n = *(ASTNode**)at(c->subnodes, i);
    concat(c->value, newString(n->value->content));
    deleteAST(&n);
  }
  // not legit
  c->subnodes->size = 0;
}

void astupnode(ASTNode *super, ASTNode *sub) {
  concat(super->value, newString(sub->value->content));
  if (!super->name->content[0]) {
    concat(super->name,  newString(sub->name->content));
  }
  Array *tmp = sub->subnodes;
  sub->subnodes = newArray(sizeof(ASTNode*));
  while (super->subnodes->size) deleteAST(pop(super->subnodes));
  deleteArray(&super->subnodes);
  super->subnodes = tmp;
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
  ASTNode *last = NULL;
  BNFNode *subbnf;
  int      size    = 0;
  char    *content = bnf->content;
  if (bnf->type != NODE_LEAF) size = ((Array*)bnf->content)->size;

  switch (bnf->type) {
    case NODE_ROOT:
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
    case NODE_ONE_OR_NONE:
      /// UNIMPLEMENTED
      asterror(errors, ERROR_UNIMPLEMENTED, bnf);
      break;
    case NODE_LEAF:
      if (c == AST_LOCK) {
        if (!content)       { ast->status = STATUS_CONFIRMED; break; }
        else if (!ast->pos) { 
          if (ast->status == STATUS_NOSTATUS) { ast->status = STATUS_ONGOING;   break; }
          else                                { ast->status = STATUS_FAILED;    break; }
        }
      } else if (c == AST_CLOSE) {
        if (!content)       { ast->status = STATUS_CONFIRMED; break; }
        else if (!ast->pos) { ast->status = STATUS_FAILED;    break; }
        c = 0;
      }
      if (content && content[ast->pos] == c) {
        if (!c || (concatmode && !content[ast->pos + 1])) {
          ast->status = STATUS_CONFIRMED;
          concat(ast->value, newString(bnf->content));
        } else {
          ast->status = STATUS_ONGOING;
        }
        ast->pos++;
      } else {
        ast->status = STATUS_FAILED;
      }
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
        astnewchar(errors, subast, subbnf, reserved, concatmode || bnf->type == NODE_CONCAT, c);
        if (subast->status == STATUS_FAILED) {
          deleteAST(rem(superast->subnodes, i--)); lim--;
        } else if (subast->status == STATUS_PARTIAL) {
          ASTNode *nast = newASTNode(superast, bnf);
          for (int j = 0; j < ast->pos; j++) {
            ASTNode *confirmed = duplicateAST(astsubnode(ast, j));
            push(nast->subnodes, &confirmed);
          }
          for (int j = 0; j < subast->subnodes->size; j++) {
            ASTNode *partial = astsubnode(subast, j);
            if (partial->status == STATUS_CONFIRMED) {
              rem(subast->subnodes, j);
              push(nast->subnodes, &partial);
              break;
            }
          }
          if ((nast->pos = ast->pos + 1) == size) {
            nast->status = STATUS_CONFIRMED;
            last = nast;
          } else if (c == AST_CLOSE || c == AST_LOCK) lim++;
        } else if (subast->status == STATUS_CONFIRMED) {
          if (++ast->pos == size) {
            ast->status = STATUS_CONFIRMED;
            last = ast;
            if (bnf->type == NODE_CONCAT) {
              //concat node
            }
          } else if (c == AST_CLOSE || c == AST_LOCK) i--;
        }
      }
      if (last) {
        astcheckrecnode(last);
        if (last->subnodes->size == 1) {
          astupnode(last, astsubnode(last, 0));
        }
        if (superast->subnodes->size == 1) {
          astupnode(superast, last);
          superast->status = STATUS_CONFIRMED;
        } else {
          superast->status = STATUS_PARTIAL;
        }
      }
      if (!superast->subnodes->size) {
        superast->status = STATUS_FAILED;
      }
      break;
    case NODE_ONE_OF:
      superast = ast;
      if (!ast->subnodes->size) ast = newASTNode(superast, bnf);
      else                      ast = astsubnode(superast, 0);
      superast->status = STATUS_ONGOING;
      ast->status = STATUS_ONGOING;
      //////////////////////////////////////////////////
      for (int i = 0; i < size; i++) {
        subast = astsubnode(ast, i);
        subbnf = bnfsubnode(bnf, i);
        if (!subast) subast = newASTNode(ast, subbnf);
        if (subast->status != STATUS_FAILED) {
          astnewchar(errors, subast, subbnf, reserved, concatmode, c);
          if (subast->status == STATUS_CONFIRMED)    { ast->status = STATUS_CONFIRMED; last = subast; } 
          else if (subast->status == STATUS_PARTIAL) { ast->status = STATUS_PARTIAL;   last = subast; }
          else if (subast->status == STATUS_FAILED)  { ast->pos++;                                    }
        }
      }
      if (ast->status == STATUS_CONFIRMED && ast->pos == size - 1) {
        superast->status = STATUS_CONFIRMED;
        astupnode(superast, last);
        break;
      } else if (ast->status == STATUS_CONFIRMED) {
        ASTNode *nast    = duplicateAST(last);
        superast->status = STATUS_PARTIAL;
        ast->status      = STATUS_ONGOING;
        last->status     = STATUS_FAILED;
        concat(nast->name, newString(ast->name->content));
        push(superast->subnodes, &nast);
        ast->pos++;
      } else if (ast->status == STATUS_PARTIAL) {
        //ASTNode *nast = newASTNode(superast, bnf);
        superast->status = STATUS_PARTIAL;
        for (int i = 0; i < last->subnodes->size; i++) {
          ASTNode *partial = astsubnode(last, i);
          if (partial->status == STATUS_CONFIRMED) {
            rem(last->subnodes, i);
            concat(partial->name, newString(ast->name->content));
            push(superast->subnodes, &partial);
            break;
          }
        }
      } else if (ast->pos >= size) {
        // should work, to verify
        deleteAST(pop(superast->subnodes));
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
  astnewchar(errors, ast, rootent, reserved, 0, AST_LOCK);

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