#include <ast.h>

void astnewchar(Array*, ASTNode*, BNFNode*, Array*, ASTFlags, char);
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
    new->rec      = 0;
    new->mod      = 0;
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
    new->rec      = ast->rec;
    new->mod      = ast->mod;
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
  while(c->subnodes->size) {
    ASTNode *n = *(ASTNode**)rem(c->subnodes, 0);
    concat(c->value, newString(n->value->content));
    deleteAST(&n);
  }
}

void astupnode(ASTNode *super, ASTNode *sub) {
  concat(super->value, newString(sub->value->content));
  if (!super->name->content[0]) {
    super->ref = sub->ref;
    concat(super->name, newString(sub->name->content));
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
  for (int i = 0; i < node->subnodes->size; i++) {
    ASTNode *n = astsubnode(node, i);
    if (n->ref->type == NODE_REC) {
      if (n->subnodes->size) {
        astcheckrecnode(n);
        set(node->subnodes, i, pop(n->subnodes));
        deleteAST(&n);
      } else {
        deleteAST(rem(node->subnodes, i--));
      }
    }
  }
}

void astnewchar(Array *errors, ASTNode *ast, BNFNode *bnf, Array *reserved, ASTFlags flags, char c) {
  ASTNode *superast;
  ASTNode *subast;
  ASTNode *save = NULL;
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
        else if (!ast->pos) { ast->status = STATUS_ONGOING;   break; }
      } else if (c == AST_CLOSE) {
        if (!content)       { ast->status = STATUS_CONFIRMED; break; }
        else if (!ast->pos) { ast->status = STATUS_FAILED;    break; }
        c = 0;
      }
      if (content && content[ast->pos] == c) {
        if (!c || ((flags & MODE_CONCAT) && !content[ast->pos + 1])) {
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
        subast = astsubnode(ast, ast->pos + ast->rec);
        subbnf = bnfsubnode(bnf, ast->pos);
        if (!subast) subast = newASTNode(ast, subbnf);
        int f = (flags & ~MODE_REC) | (bnf->type == NODE_CONCAT ? MODE_CONCAT : 0);
        astnewchar(errors, subast, subbnf, reserved, f, c);
        if (subast->status == STATUS_FAILED) { 
          deleteAST(rem(superast->subnodes, i--)); lim--;
        } else if (subast->status == STATUS_PARTIAL) {
          ASTNode *nast = newASTNode(superast, bnf);
          for (int j = 0; j < ast->pos + ast->rec; j++) {
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
          nast->rec = ast->rec;
          if ((nast->pos = ast->pos + 1) == size) {
            nast->status = STATUS_CONFIRMED;
            save = nast;
          } else if (c == AST_CLOSE || c == AST_LOCK) {
            lim++;
          }
        } else if (subast->status == STATUS_CONFIRMED) {
          if (++ast->pos == size) {
            ast->status = STATUS_CONFIRMED;
            save = ast;
          } else if (c == AST_CLOSE || c == AST_LOCK) { 
            i--;
          }
        } else if (subast->mod) {
          ASTNode *rec       = astsubnode(subast, 0);
          ASTNode *superlist = astsubnode(rec, 0);
          ASTNode *list      = astsubnode(superlist, 0);
          if (superlist->subnodes->size == 1) {
            int l = list->subnodes->size;
            ASTNode *n = *(ASTNode**)pop(ast->subnodes);
            while (list->subnodes->size) {
              insert(ast->subnodes, ast->pos + ast->rec++, rem(list->subnodes, 0));
            }
            deleteAST(&n);
            if (l > 1) ast->rec--;
          }
        }
        if ((flags & MODE_REC) && ast->pos == size - 1 && superast->subnodes->size == 1) {
          ast->mod      = 1;
          superast->mod = 1;
        }
      }
      if (save) {
        astcheckrecnode(save);
        if (save->subnodes->size == 1) {
          //astupnode(save, astsubnode(save, 0));
        } else if (bnf->type == NODE_CONCAT) {
          astconcatnode(save);
        }
        if (superast->subnodes->size == 1) {
          astupnode(superast, save);
          superast->status = STATUS_CONFIRMED;
          break;
        } else {
          superast->status = STATUS_PARTIAL;
        }
      }
      if (!superast->subnodes->size) {
        superast->status = STATUS_FAILED;
      }
      break;
    case NODE_REC:
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
          int f = (flags & ~MODE_REC) | (bnf->type == NODE_REC ? MODE_REC : 0);
          astnewchar(errors, subast, subbnf, reserved, f, c);
          if (subast->status == STATUS_CONFIRMED)    { ast->status = STATUS_CONFIRMED; save = subast; }
          else if (subast->status == STATUS_PARTIAL) { ast->status = STATUS_PARTIAL;   save = subast; }
          else if (subast->status == STATUS_FAILED)  { ast->pos++;                                    }
          if (bnf->type == NODE_REC) superast->mod |= ast->mod |= subast->mod;
        }
      }
      if (ast->status == STATUS_CONFIRMED && ast->pos == size - 1) {
        superast->status = STATUS_CONFIRMED;
        astupnode(superast, save);
        break;
      } else if (ast->status == STATUS_CONFIRMED) {
        ASTNode *nast    = duplicateAST(save);
        superast->status = STATUS_PARTIAL;
        ast->status      = STATUS_ONGOING;
        save->status     = STATUS_FAILED;
        if (!nast->name->length) {
          nast->ref = ast->ref;
          concat(nast->name, newString(ast->name->content));
        }
        push(superast->subnodes, &nast);
        ast->pos++;
      } else if (ast->status == STATUS_PARTIAL) {
        superast->status = STATUS_PARTIAL;
        for (int i = 0; i < save->subnodes->size; i++) {
          ASTNode *partial = astsubnode(save, i);
          if (partial->status == STATUS_CONFIRMED) {
            rem(save->subnodes, i);
            if (!partial->name->length) {
              partial->ref = ast->ref;
              concat(partial->name, newString(ast->name->content));
            }
            push(superast->subnodes, &partial);
            break;
          }
        }
      } else if (ast->pos >= size) {
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
  //astnewchar(errors, ast, rootent, reserved, 0, AST_CLOSE);

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