#include <ast.h>

ASTNode *astsubnode(ASTNode*, int);

ASTNode *newASTNode(ASTNode *ast, BNFNode *bnf)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name     = newString(bnf ? bnf->name : "");
    new->ref      = bnf;
    new->subnodes = newArray(sizeof(ASTNode*));
    new->value    = newString("");
    new->status   = STATUS_NOSTATUS;
    new->pos      = 0;
    new->rec      = 0;
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
    if (new->subnodes) {
      for (int i = 0; i < ast->subnodes->size; i++) {
        ASTNode *n = duplicateAST(astsubnode(ast, i));
        push(new->subnodes, &n);
      }
    }
  }
  return new;
}

ASTNode *astsubnode(ASTNode *ast, int index)
{
  ASTNode **ret = (ASTNode**)at(ast->subnodes, index);
  if (ret) return *ret;
  return NULL;
}

int astempty(ASTNode *ast) {
  return !ast->subnodes->size && !ast->value->length;
}

BNFNode *bnfsubnode(BNFNode *bnf, int index)
{
  return *(BNFNode**)at(bnf->content, index);
}

void freeastnode(ASTNode *node)
{
  deleteString(&node->name);
  deleteString(&node->value);
  deleteArray(&node->subnodes);
}

void astconcatnode(ASTNode *c)
{
  while(c->subnodes->size) {
    ASTNode *n = *(ASTNode**)rem(c->subnodes, 0);
    concat(c->value, newString(n->value->content));
    deleteAST(&n);
  }
}

void astupnode(ASTNode *super, ASTNode *sub)
{
  concat(super->value, newString(sub->value->content));
  if (!super->name->content[0]) {
    super->ref = sub->ref;
    concat(super->name, newString(sub->name->content));
  } else if (sub->name->length) {
    String *tmp = super->name;
    super->name = sub->name;
    sub->name = tmp;
  }
  Array *tmp = sub->subnodes;
  sub->subnodes = newArray(sizeof(ASTNode*));
  while (super->subnodes->size) deleteAST(pop(super->subnodes));
  deleteArray(&super->subnodes);
  super->subnodes = tmp;
}

void asterror(Array *errors, ASTErrorType errno, BNFNode *bnf)
{
  ASTError error;
  error.errno  = errno;
  error.bnfref = bnf;
  push(errors, &error);
}

void astnewchar(ASTNode *ast, BNFNode *bnf, ASTFlags flags, char c)
{
  ASTNode  *superast;
  ASTNode  *subast;
  ASTNode  *save    = NULL;
  BNFNode  *subbnf;
  int       size    = 0;
  int       f       = 0;
  char     *content = bnf->content;
  if (bnf->type != NODE_LEAF && bnf->type != NODE_RAW) size = ((Array*)bnf->content)->size;
  push(bnf->refs, &ast);

  switch (bnf->type) {
    case NODE_ROOT:
    case NODE_RAW:
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
    case NODE_ONE_OR_NONE:
      /// UNIMPLEMENTED
      break;
    case NODE_LEAF:
      if (c == AST_LOCK) {
        if (!content || ast->status == STATUS_CONFIRMED) { ast->status = STATUS_CONFIRMED; break; }
        else if (!ast->pos && (flags & ASTFLAGS_STARTED
                            || flags & ASTFLAGS_END))    { ast->status = STATUS_FAILED;    break; }
        else if (!ast->pos)                              { ast->status = STATUS_ONGOING;   break; }
      }
      if (content && content[ast->pos] == c) {
        if (!c || ((flags & ASTFLAGS_CONCAT) && !content[ast->pos + 1])) {
          ast->status = STATUS_CONFIRMED;
          concat(ast->value, newString(bnf->content));
        } else { 
          ast->status = STATUS_ONGOING;
          ast->pos++;
        }
      } else { ast->status = STATUS_FAILED; }
      break;
    case NODE_CONCAT:
    case NODE_LIST:
    //////////////////////////////// LIST HEADER /////////////////////////////////////////
      superast = ast;
      if (superast->status == STATUS_FAILED) break;
      if (superast->status == STATUS_NOSTATUS) superast->status = STATUS_ONGOING;
      if (superast->status == STATUS_CONFIRMED) {
        if (c != AST_LOCK) { 
          while (superast->subnodes->size) deleteAST(pop(superast->subnodes));
          superast->status = STATUS_FAILED;
        }
        break;
      }
      if (!superast->subnodes->size) {
        ast = newASTNode(superast, bnf);
        ast->status = STATUS_ONGOING;
      }
      if (bnf->type == NODE_CONCAT) {
        if (c == AST_LOCK)       { if (superast->pos) superast->pos++; }
        else if (!superast->pos) {                    superast->pos++; }
      }
      for (int i = 0; i < superast->subnodes->size; i++) {
        ast = astsubnode(superast, i);
        ///////////////////////////// LIST BODY ////////////////////////////////////////////
        for (char nc = c;;) {
          if (ast->pos >= size) {
            ast->status = STATUS_CONFIRMED;
            save = ast;
            break;
          } else if (ast->pos >= size - 1 && flags & ASTFLAGS_REC) {
            ast->status = STATUS_REC;
            save = ast;
            break;
          } else if (ast->status == STATUS_NOSTATUS) {
            ast->status = STATUS_ONGOING;
            nc = AST_LOCK;
          }
          subast = astsubnode(ast, ast->pos + ast->rec);
          subbnf = bnfsubnode(bnf, ast->pos);
          if (!subast) subast = newASTNode(ast, NULL);
          f |= (flags & ~ASTFLAGS_REC) | (bnf->type == NODE_CONCAT ? ASTFLAGS_CONCAT : 0);
          f |= superast->pos > 1 ? ASTFLAGS_STARTED : 0;
          f |= !ast->pos ? ASTFLAGS_FRONT : 0;
          astnewchar(subast, subbnf, f, nc);
          if (subast->status == STATUS_FAILED) {
            deleteAST(rem(superast->subnodes, i--));
          } else if (subast->status == STATUS_PARTIAL) {
            ASTNode *nast = newASTNode(superast, bnf);
            subast->status = STATUS_ONGOING; // STEAL PARTIAL AWAY
            nast->status   = STATUS_NOSTATUS;
            nast->rec = ast->rec;
            nast->pos = ast->pos;
            for (int j = 0; j < ast->pos + ast->rec; j++) {
              ASTNode *confirmed = duplicateAST(astsubnode(ast, j));
              push(nast->subnodes, &confirmed);
            }
            for (int j = 0; j < subast->subnodes->size; j++) {
              ASTNode *partial = astsubnode(subast, j);
              int      status  = partial->status;
              if (status == STATUS_CONFIRMED || status == STATUS_REC) {     
                if (!subast->rec) rem(subast->subnodes, j);
                else partial = duplicateAST(partial);
                if (status == STATUS_CONFIRMED && astempty(partial)) {
                  deleteAST(&partial);
                  nast->rec--;
                } else { push(nast->subnodes, &partial); }
                if (status == STATUS_REC) nast->rec++;
                else                      nast->pos++;
                break;
              }
            }
          } else if (subast->status == STATUS_CONFIRMED) {
            if (astempty(subast)) {
              ast->rec--;
              deleteAST(pop(ast->subnodes));
            }
            ++ast->pos;
            nc = AST_LOCK;
            continue;
          } else if (subast->status == STATUS_REC) {
            ++ast->rec;
            nc = AST_LOCK;
            continue;
          } else if (subast->status == STATUS_SKIP)  {
            ASTNode *nast = newASTNode(superast, bnf);
            push(nast->subnodes, pop(subast->subnodes));
            subast->status = STATUS_ONGOING;
            nc = AST_LOCK;
            continue;
          }
          break;
        }
      }
      /////////////////////////// LIST FOOTER ///////////////////////////////////////////
      if (save) {
        if (save->subnodes->size == 1)     astupnode(save, astsubnode(save, 0));
        else if (bnf->type == NODE_CONCAT) astconcatnode(save);
        if (superast->subnodes->size == 1) {
          astupnode(superast, save);
          superast->status = ast->status;
          break;
        } else { superast->status = STATUS_PARTIAL; }
      }
      if (!superast->subnodes->size) superast->status = STATUS_FAILED;
      break;
      ///////////////////////////////////////////////////////////////////////////////////
    case NODE_REC:
    case NODE_ONE_OF:
    case NODE_ANON:
    /////////////////////////////// ONE OF HEADER ///////////////////////////////////////
      superast = ast;
      if (bnf->refs->size > 1 && flags & ASTFLAGS_FRONT) {
        // SPECIAL CASE OF LEFTMOST DERIVATION (when the recursive element is the first position)
        ASTNode *recroot = *(ASTNode**)at(bnf->refs, 0);
        recroot->rec = 1;
        if (flags & ASTFLAGS_END) { superast->status = STATUS_FAILED; break; }
        if (recroot->status == STATUS_PARTIAL && c == AST_LOCK) {
          recroot->status  = STATUS_ONGOING;
          recroot->pos     = bnf->refs->size;
          ASTNode *partial = *(ASTNode**)pop(recroot->subnodes);
          push(superast->subnodes, &partial);
          superast->status = STATUS_SKIP;
        }
        break;
      }
      if (superast->status == STATUS_FAILED || superast->status == STATUS_CONFIRMED) break;
      if (superast->status == STATUS_NOSTATUS) superast->status = STATUS_ONGOING;
      if (!superast->subnodes->size) ast = newASTNode(superast, bnf);
      else                           ast = astsubnode(superast, 0);
      ast->status = STATUS_ONGOING;
      ///////////////////////////// ONE OF BODY //////////////////////////////////////////
      for (int i = 0; i < size; i++) {
        subast = astsubnode(ast, i);
        subbnf = bnfsubnode(bnf, i);
        if (!subast) subast = newASTNode(ast, NULL);
        if (subast->status != STATUS_FAILED) {
          f |= (flags & ~ASTFLAGS_REC) | (bnf->type == NODE_REC ? ASTFLAGS_REC : 0);
          astnewchar(subast, subbnf, f, c);
          if (subast->status == STATUS_CONFIRMED)    { ast->status = STATUS_CONFIRMED; save = subast; }
          else if (subast->status == STATUS_PARTIAL) { ast->status = STATUS_PARTIAL;   save = subast; }
          else if (subast->status == STATUS_REC)     { ast->status = STATUS_REC;       save = subast; }
          else if (subast->status == STATUS_FAILED)  { ast->pos++;                                    }
        }
      }
      if (c != AST_LOCK && superast->subnodes->size > 1) deleteAST(pop(superast->subnodes));
      if ((ast->status == STATUS_CONFIRMED && ast->pos == size - 1) || ast->status == STATUS_REC) {
        superast->status = ast->status;
        if (bnf->type == NODE_ANON) {
          astupnode(superast, save);
        } else {
          astupnode(ast,     save); // for the name
          astupnode(superast, ast);
        }
        break;
      } else if (ast->status == STATUS_CONFIRMED) {
        ASTNode *nast    = newASTNode(NULL, NULL);
        superast->status = STATUS_PARTIAL;
        ast->status      = STATUS_ONGOING;
        nast->status     = STATUS_FAILED;
        set(ast->subnodes, indexof(ast->subnodes, &save), &nast);
        if (!save->name->length && bnf->type != NODE_ANON) concat(save->name, newString(ast->name->content));
        push(superast->subnodes, &save);
        ast->pos++;
      } else if (ast->status == STATUS_PARTIAL) {
        superast->status = STATUS_PARTIAL;
        ast->status      = STATUS_ONGOING; // STEAL PARTIAL AWAY
        save->status     = STATUS_ONGOING;
        for (int i = 0; i < save->subnodes->size; i++) {
          ASTNode *partial = astsubnode(save, i);
          if (partial->status == STATUS_CONFIRMED || partial->status == STATUS_REC) {
            if (!save->rec) rem(save->subnodes, i);
            else partial = duplicateAST(partial);
            if (!partial->name->length && bnf->type != NODE_ANON) {
              partial->ref = ast->ref;
              concat(partial->name, newString(ast->name->content));
            }
            push(superast->subnodes, &partial);
            break;
          }
        }
      } else if (ast->pos >= size) {
        deleteAST(rem(superast->subnodes, 0));
        if (!superast->subnodes->size) superast->status = STATUS_FAILED;
        // in case the AST was partial
        else {
          // Memory leak?
          astupnode(superast, astsubnode(superast, 0));
          superast->status = STATUS_CONFIRMED;
        }
      }
      break;
  }
  pop(bnf->refs);
}

void astnewsymbol(ASTNode *ast, BNFNode *bnf, BNFNode *raw, ASTFlags flags, Symbol *s)
{
  ASTNode  *superast;
  ASTNode  *subast;
  ASTNode  *save    = NULL;
  BNFNode  *subbnf;
  int       size    = 0;
  int       f       = 0;
  char     *content = bnf->content;
  if (bnf->type != NODE_LEAF && bnf->type != NODE_RAW) size = ((Array*)bnf->content)->size;
  push(bnf->refs, &ast);

  switch (bnf->type) {
    case NODE_ROOT:
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
    case NODE_ONE_OR_NONE:
    case NODE_CONCAT:
      /// UNIMPLEMENTED
      break;
    case NODE_RAW:
      if (s) {
        if (s->type == (SymbolType)bnf->content) {
          if (s->type == SYMBOL_NUMBER) {
            ASTNode *number = newASTNode(NULL, NULL);
            astnewchar(number, raw, ASTFLAGS_NONE, AST_LOCK);
            for (int i = 0; s->text[i]; i++) astnewchar(number, raw, ASTFLAGS_NONE, s->text[i]);
            astnewchar(number, raw, ASTFLAGS_END, AST_LOCK);
            if (number->status == STATUS_CONFIRMED) {
              ast->status = STATUS_CONFIRMED;
              concat(ast->value, newString(number->value->content));
              concat(ast->name,  newString(number->name->content));
            } else {
              ast->status = STATUS_FAILED;
            }
            deleteAST(&number);
          } else {
            concat(ast->value, newString(s->text));
            ast->status = STATUS_CONFIRMED;
          }
        } else ast->status = STATUS_FAILED;
      } else {
        ast->status = STATUS_ONGOING;
      }
      break;
    case NODE_LEAF:
      if (s) {
        if (content) {
          if (strcmp(s->text, content)) {
            ast->status = STATUS_FAILED;
          } else {
            concat(ast->value, newString(content)); ast->status = STATUS_CONFIRMED;
          }
        }
      } else {
        if (content && ast->status != STATUS_CONFIRMED) {
          if (flags & ASTFLAGS_END) ast->status = STATUS_FAILED;
          else                      ast->status = STATUS_ONGOING;
        } else                      ast->status = STATUS_CONFIRMED;
      }
      break;
    case NODE_LIST:
    //////////////////////////////// LIST HEADER /////////////////////////////////////////
      superast = ast;
      if (superast->status == STATUS_FAILED) break;
      if (superast->status == STATUS_NOSTATUS) superast->status = STATUS_ONGOING;
      if (superast->status == STATUS_CONFIRMED) {
        if (s){
          while (superast->subnodes->size) deleteAST(pop(superast->subnodes));
          superast->status = STATUS_FAILED;
        }
        break;
      }
      if (!superast->subnodes->size) {
        ast = newASTNode(superast, bnf);
        ast->status = STATUS_ONGOING;
      }
      for (int i = 0; i < superast->subnodes->size; i++) {
        ast = astsubnode(superast, i);
        ///////////////////////////// LIST BODY ////////////////////////////////////////////
        for (Symbol *ns = s;;) {
          if (ast->pos >= size) {
            ast->status = STATUS_CONFIRMED;
            save = ast;
            break;
          } else if (ast->pos >= size - 1 && flags & ASTFLAGS_REC) {
            ast->status = STATUS_REC;
            save = ast;
            break;
          } else if (ast->status == STATUS_NOSTATUS) {
            ast->status = STATUS_ONGOING;
            ns = NULL;
          }
          subast = astsubnode(ast, ast->pos + ast->rec);
          subbnf = bnfsubnode(bnf, ast->pos);
          if (!subast) subast = newASTNode(ast, NULL);
          f |= flags & ~ASTFLAGS_REC;
          f |= !ast->pos ? ASTFLAGS_FRONT : 0;
          astnewsymbol(subast, subbnf, raw, f, ns);
          if (subast->status == STATUS_FAILED) {
            deleteAST(rem(superast->subnodes, i--)); break;
          } else if (subast->status == STATUS_PARTIAL) {
            ASTNode *nast = newASTNode(superast, bnf);
            subast->status = STATUS_ONGOING; // STEAL PARTIAL AWAY
            nast->status   = STATUS_NOSTATUS;
            nast->rec = ast->rec;
            nast->pos = ast->pos;
            for (int j = 0; j < ast->pos + ast->rec; j++) {
              ASTNode *confirmed = duplicateAST(astsubnode(ast, j));
              push(nast->subnodes, &confirmed);
            }
            for (int j = 0; j < subast->subnodes->size; j++) {
              ASTNode *partial = astsubnode(subast, j);
              int      status  = partial->status;
              if (status == STATUS_CONFIRMED || status == STATUS_REC) {
                if (!subast->rec) rem(subast->subnodes, j);
                else partial = duplicateAST(partial);
                if (status == STATUS_CONFIRMED && astempty(partial)) {
                  deleteAST(&partial);
                  nast->rec--;
                } else { push(nast->subnodes, &partial); }
                if (status == STATUS_REC) nast->rec++;
                else                      nast->pos++;
                break;
              }
            }
          } else if (subast->status == STATUS_CONFIRMED) {
            if (astempty(subast)) {
              ast->rec--;
              deleteAST(pop(ast->subnodes));
            }
            ++ast->pos;
          } else if (subast->status == STATUS_REC) {
            ++ast->rec;
          } else if (subast->status == STATUS_SKIP)  {
            ASTNode *nast = newASTNode(superast, bnf);
            push(nast->subnodes, pop(subast->subnodes));
            subast->status = STATUS_ONGOING;
          } else if (!ns) break;
          if (ns) ns = NULL;
        }
      }
      /////////////////////////// LIST FOOTER ///////////////////////////////////////////
      if (save) {
        if (save->subnodes->size == 1) astupnode(save, astsubnode(save, 0));
        if (superast->subnodes->size == 1) {
          astupnode(superast, save);
          superast->status = ast->status;
          break;
        } else { superast->status = STATUS_PARTIAL; }
      }
      if (!superast->subnodes->size) superast->status = STATUS_FAILED;
      break;
      ///////////////////////////////////////////////////////////////////////////////////
    case NODE_REC:
    case NODE_ONE_OF:
    case NODE_ANON:
    /////////////////////////////// ONE OF HEADER ///////////////////////////////////////
      superast = ast;
      if (bnf->refs->size > 1 && flags & ASTFLAGS_FRONT) {
        // SPECIAL CASE OF LEFTMOST DERIVATION (when the recursive element is the first position)
        ASTNode *recroot = *(ASTNode**)at(bnf->refs, bnf->refs->size - 2);
        recroot->rec = 1;
        if (flags & ASTFLAGS_END) { superast->status = STATUS_FAILED; break; }
        if (recroot->subnodes->size > 1 && !s) {
          recroot->status  = STATUS_ONGOING;
          recroot->pos     = bnf->refs->size;
          ASTNode *partial = *(ASTNode**)pop(recroot->subnodes);
          push(superast->subnodes, &partial);
          superast->status = STATUS_SKIP;
        }
        break;
      }
      if (superast->status == STATUS_FAILED || superast->status == STATUS_CONFIRMED) break;
      if (superast->status == STATUS_NOSTATUS) superast->status = STATUS_ONGOING;
      if (!superast->subnodes->size) ast = newASTNode(superast, bnf);
      else                           ast = astsubnode(superast, 0);
      ast->status = STATUS_ONGOING;
      ///////////////////////////// ONE OF BODY //////////////////////////////////////////
      for (int i = 0; i < size; i++) {
        subast = astsubnode(ast, i);
        subbnf = bnfsubnode(bnf, i);
        if (!subast) subast = newASTNode(ast, NULL);
        if (subast->status != STATUS_FAILED) {
          f |= (flags & ~ASTFLAGS_REC) | (bnf->type == NODE_REC ? ASTFLAGS_REC : 0);
          astnewsymbol(subast, subbnf, raw, f, s);
          if (subast->status == STATUS_CONFIRMED)    { ast->status = STATUS_CONFIRMED; save = subast; }
          else if (subast->status == STATUS_PARTIAL) { ast->status = STATUS_PARTIAL;   save = subast; }
          else if (subast->status == STATUS_REC)     { ast->status = STATUS_REC;       save = subast; }
          else if (subast->status == STATUS_FAILED)  { ast->pos++;                                    }
        }
      }
      if (s && superast->subnodes->size > 1) deleteAST(pop(superast->subnodes));
      if ((ast->status == STATUS_CONFIRMED && ast->pos == size - 1) || ast->status == STATUS_REC) {
        superast->status = ast->status;
        if (bnf->type == NODE_ANON) {
          astupnode(superast, save);
        } else {
          astupnode(ast,     save); // for the name
          astupnode(superast, ast);
        }
        break;
      } else if (ast->status == STATUS_CONFIRMED) {
        ASTNode *nast    = newASTNode(NULL, NULL);
        superast->status = STATUS_PARTIAL;
        ast->status      = STATUS_ONGOING;
        nast->status     = STATUS_FAILED;
        set(ast->subnodes, indexof(ast->subnodes, &save), &nast);
        if (!save->name->length && bnf->type != NODE_ANON) concat(save->name, newString(ast->name->content));
        push(superast->subnodes, &save);
        ast->pos++;
      } else if (ast->status == STATUS_PARTIAL) {
        superast->status = STATUS_PARTIAL;
        ast->status      = STATUS_ONGOING; // STEAL PARTIAL AWAY
        save->status     = STATUS_ONGOING;
        for (int i = 0; i < save->subnodes->size; i++) {
          ASTNode *partial = astsubnode(save, i);
          if (partial->status == STATUS_CONFIRMED || partial->status == STATUS_REC) {
            if (!save->rec) rem(save->subnodes, i);
            else partial = duplicateAST(partial);
            if (!partial->name->length && bnf->type != NODE_ANON) {
              partial->ref = ast->ref;
              concat(partial->name, newString(ast->name->content));
            }
            push(superast->subnodes, &partial);
            break;
          }
        }
      } else if (ast->pos >= size) {
        deleteAST(rem(superast->subnodes, 0));
        if (!superast->subnodes->size) superast->status = STATUS_FAILED;
        // in case the AST was partial
        else {
          // Memory leak?
          astupnode(superast, astsubnode(superast, 0));
          superast->status = STATUS_CONFIRMED;
        }
      }
      break;
  }
  pop(bnf->refs);
}

ASTNode *parseast(char *filename)
{
  BNFNode      *bnftree  = parsebnf("parsing/bnf/preprocessor.bnf");
  BNFNode      *rawtree  = parsebnf("parsing/bnf/raw.bnf");
  BNFNode      *rootent  = bnfsubnode(bnftree, 0);
  BNFNode      *rawent   = bnfsubnode(rawtree, 0);
  Parser       *parser   = newParser("parsing/prs/csr.prs");
  SymbolStream *ss       = ssopen(filename, parser);
  ASTNode      *ast      = newASTNode(NULL, NULL);
  Array        *trace    = newArray(sizeof(char*));
  push(trace, &filename);

  Symbol *s;
  while (!(s = ssgets(ss))->eof) {
    if (s->text[0] == '\n') continue;
    astnewsymbol(ast, rootent, rawent, ASTFLAGS_NONE, NULL);
    astnewsymbol(ast, rootent, rawent, ASTFLAGS_NONE, s);
    if (ast->status == STATUS_FAILED) {
      printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected symbol!");
      break;
    }
  }
  //astnewsymbol(ast, rootent, rawent, ASTFLAGS_END, NULL);

  deleteArray(&trace);
  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&bnftree);
  deleteBNFTree(&rawtree);
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