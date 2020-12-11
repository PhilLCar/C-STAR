#include <ast.h>

#include <string.h>
#include <stdlib.h>

#include <parser.h>
#include <strings.h>
#include <error.h>

void freeastnode(ASTNode *node)
{
  deleteString(&node->name);
  deleteString(&node->value);
  deleteArray(&node->subnodes);
  deleteSymbol(&node->symbol);
}

ASTNode *newASTNode(ASTNode *ast, BNFNode *bnf)
{
  ASTNode *new = malloc(sizeof(ASTNode));
  if (new) {
    new->name          = newString(bnf ? bnf->name : "");
    new->ref           = bnf;
    new->subnodes      = newArray(sizeof(ASTNode*));
    new->value         = newString("");
    new->symbol        = NULL;
    new->scope         = 0;
    new->continuation  = 0;
    new->recurse       = 0;
    new->newline       = 0;
    if (ast) push(ast->subnodes, &new);
  }
  return new;
}

void deleteAST(ASTNode **node) {
  if (*node) {
    while((*node)->subnodes->size) deleteAST(pop((*node)->subnodes));
    freeastnode(*node);
    free(*node);
    *node = NULL;
  }
}

void revertAST(ASTNode **node, Stream *s) {
  if (*node) {
    if ((*node)->recurse) (*node)->recurse = 0;
    else {
      while ((*node)->subnodes->size) revertAST(pop((*node)->subnodes), s);
      if ((*node)->newline) {
        Symbol newline = { malloc(2 * sizeof(char)), NULL, NULL, 0, 0, SYMBOL_NEWLINE };
        sprintf(newline.text, "\n");
        if (s->symbol->text) {
          push(s->stack, s->symbol);
          memset(s->symbol, 0, sizeof(Symbol));
        }
        push(s->stack, &newline);
      }
      if ((*node)->symbol) {
        if (s->symbol->text) {
          push(s->stack, s->symbol);
          memset(s->symbol, 0, sizeof(Symbol));
        }
        push(s->stack, (*node)->symbol);
        free((*node)->symbol);
      }
      (*node)->symbol = NULL;
      freeastnode(*node);
      free(*node);
      *node = NULL;
    }
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
  Symbol *t   = sub->symbol;
  Array  *tmp = sub->subnodes;
  sub->subnodes = newArray(sizeof(ASTNode*));
  sub->symbol = super->symbol;
  while (super->subnodes->size) deleteAST(pop(super->subnodes));
  deleteArray(&super->subnodes);
  super->subnodes = tmp;
  super->symbol   = t;
}

void astflatten(ASTNode *super, int index)
{
  ASTNode *sub = *(ASTNode**)at(super->subnodes, index);
  while (sub->subnodes->size) {
    insert(super->subnodes, index + 1, pop(sub->subnodes));
  }
  deleteAST(rem(super->subnodes, index));
}

int isopening(Symbol *symbol, Parser *parser)
{
  for (int i = 0; parser->delimiters[i]; i++) {
    if (!strcmp(symbol->text, parser->delimiters[i])) {
      return (i + 1) % 2;
    }
  }
  return 0;
}

void astnextsymbol(Stream *s) {
  while (s->gets(s->stream) && (s->symbol->type == SYMBOL_COMMENT || s->symbol->type == SYMBOL_NEWLINE));
}

ASTStatus _astparsestream(ASTNode *ast, BNFNode *bnf, Array *rejected, int recurse, Stream *s)
{
  BNFNode   *subbnf;
  ASTNode   *subast;
  ASTStatus  status  = STATUS_FAILED;
  Symbol    *symbol  = s->symbol;
  int        size    = 0;
  char      *content = bnf->content;
  int        partial = -1;

  // Special cases
  if (rejected && in(rejected, &bnf)) {
    return STATUS_FAILED;
  }
  if (bnf->refs->size) {
    ASTNode *recnode = *(ASTNode**)last(bnf->refs);
    if (recnode->scope == ast->scope) {
      recnode->recurse = 1;
      if (recnode->continuation) {
        status = STATUS_PARTIAL;
        push(ast->subnodes, &recnode);
      }
      return status;
    }
  }

  if (bnf->type != NODE_LEAF && bnf->type != NODE_RAW) size = ((Array*)bnf->content)->size;

  push(bnf->refs, &ast);
  do {
    switch (bnf->type) {
    case NODE_ROOT:
    case NODE_REC:
    case NODE_CONCAT:
      /// NOT APPLICABLE (legacy)
      break;
    case NODE_PEAK:
    case NODE_PEAK_NOT:
      subbnf = *(BNFNode**)at(bnf->content, 0);
      if ((bnf->type == NODE_PEAK) == !strcmp(symbol->text, subbnf->content)) {
        status = STATUS_CONFIRMED;
      }
      break;
    case NODE_CONTIGUOUS:
      deleteString(&ast->name);
      ast->name = newString("");
      status    = *s->newline ? STATUS_FAILED : STATUS_CONFIRMED;
      break;
    case NODE_NOT:
      if (!rejected) rejected = newArray(sizeof(BNFNode*));
      push(rejected, at(bnf->content, 1));
      status = _astparsestream(ast, *(BNFNode**)at(bnf->content, 0), rejected, recurse, s);
      pop(rejected);
      if (rejected && !rejected->size) deleteArray(&rejected);
      break;
    case NODE_RAW:
      if (!recurse && symbol->type == (SymbolType)bnf->content) {
        deleteString(&ast->name);
        deleteString(&ast->value);
        ast->name   = newString(bnf->name);
        ast->value  = newString(symbol->text);
        ast->symbol = newSymbol(symbol);
        astnextsymbol(s);
        ast->newline = *s->newline;
        status = STATUS_CONFIRMED;
      }
      break;
    case NODE_LEAF:
      if (content) {
        if (!recurse && symbol->text && !strcmp(symbol->text, content)) {
          deleteString(&ast->value);
          ast->value  = newString(content);
          ast->symbol = newSymbol(symbol);
          astnextsymbol(s);
          ast->newline = *s->newline;
          status = STATUS_CONFIRMED;
        }
      } else status = STATUS_CONFIRMED;
      break;
    case NODE_LIST:
      partial  = 0;
      for (int i = 0; i < size; i++) {
        subbnf = *(BNFNode**)at(bnf->content, i);
        subast = newASTNode(ast, subbnf);
        subast->scope = ast->scope + i;
        status = _astparsestream(subast, subbnf, rejected, recurse, s);
        if (status == STATUS_PARTIAL) {
          partial = 1;
          recurse = 0;
        } else if (status == STATUS_FAILED) {
          while (ast->subnodes->size) revertAST(pop(ast->subnodes), s);
          if (!symbol->text) astnextsymbol(s);
          break;
        } else if (subast->name->length || subast->value->length || subast->subnodes->size) {
          rejected = NULL;
        }
      }
      if (status != STATUS_CONFIRMED) {
        if (partial) status = STATUS_PARTIAL;
        else         status = STATUS_FAILED;
      } else {
        ASTNode *lastnode;
        if (partial) {
          subast   = *(ASTNode**)at(ast->subnodes, 0);
          lastnode = *(ASTNode**)pop(subast->subnodes);
          set(ast->subnodes, 0, rem(lastnode->subnodes, 0));
          deleteAST(&subast);
        }
        // Remove empty nodes
        for (int i = 0; i < ast->subnodes->size; i++) {
          lastnode = *(ASTNode**)at(ast->subnodes, i);
          if ((!lastnode->name->length && !lastnode->value->length) ||
               (lastnode->symbol && lastnode->symbol->type == SYMBOL_BREAK)) {
            if (lastnode->subnodes->size) astflatten(ast, i--);
            else                          deleteAST(rem(ast->subnodes, i--));
          }
        }
        if (ast->subnodes->size == 1) {
          astupnode(ast, *(ASTNode**)last(ast->subnodes));
        }
      }
      break;
    case NODE_ONE_OF:
    case NODE_ANON:
    case NODE_ONE_OR_NONE:
      subast = newASTNode(ast, NULL);
      subast->scope = ast->scope;
      for (int i = 0; i < size; i++) {
        subbnf = *(BNFNode**)at(bnf->content, i);
        status = _astparsestream(subast, subbnf, rejected, recurse, s);
        if (status == STATUS_CONFIRMED) {
          if (bnf->name[0] && bnf->type != NODE_ANON && !subast->name->length) {
            deleteString(&subast->name);
            subast->name = newString(bnf->name);
          }
          subast->ref = bnf; // Maybe to be deleted eventually
          break;
        }
      }
      if (status == STATUS_FAILED || status == STATUS_PARTIAL) {
        if (status == STATUS_PARTIAL)           status = STATUS_FAILED;
        else if (bnf->type == NODE_ONE_OR_NONE) status = STATUS_CONFIRMED;
        deleteAST(pop(ast->subnodes));
      } else if (!ast->recurse) {
        astupnode(ast, *(ASTNode**)last(ast->subnodes));
      }
      break;
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
      do {
        subast = newASTNode(ast, NULL);
        subast->scope = ast->scope + ast->subnodes->size - 1;
        for (int i = 0; i < size; i++) {
          subbnf = *(BNFNode**)at(bnf->content, i);
          status = _astparsestream(subast, subbnf, rejected, recurse, s);
          if (status == STATUS_CONFIRMED) {
            subast->ref = bnf;
            break;
          }
        }
        if (status != STATUS_CONFIRMED) {
          deleteAST(pop(ast->subnodes));
        }
      } while (status == STATUS_CONFIRMED);
      if (!ast->subnodes->size && bnf->type == NODE_MANY_OR_ONE) status = STATUS_FAILED;
      else                                                       status = STATUS_CONFIRMED;
      break;
    }
    if (status == STATUS_FAILED && ast->continuation && ast->subnodes->size == 1) {
      // Means the recursion reached its maximum
      status = STATUS_CONFIRMED;
      astupnode(ast, *(ASTNode**)last(ast->subnodes));
      ast->recurse = 0;
    }
    ast->continuation = ast->recurse;
    ast->recurse = 0;
    rejected = NULL;
    recurse  = 1;
  } while (status == STATUS_CONFIRMED && ast->continuation);
  pop(bnf->refs);

  return status;
}

ASTStatus astparsestream(ASTNode *ast, BNFNode *bnf, Stream *s)
{
  astnextsymbol(s);
  return _astparsestream(ast, bnf, NULL, 0, s);
}

ASTNode *parseast(char *filename)
{
  BNFNode      *bnftree  = parsebnf("parsing/bnf/test.bnf");
  BNFNode      *rootent  = *(BNFNode**)at(bnftree->content, 0);
  Parser       *parser   = newParser("parsing/prs/csr.prs");
  Stream       *s        = getStreamSS(ssopen(filename, parser));
  ASTNode      *ast      = newASTNode(NULL, NULL);
  Array        *trace    = newArray(sizeof(char*));
  ASTStatus     status;

  status = astparsestream(ast, rootent, s);
  if (s->symbol->type != SYMBOL_EOF || status == STATUS_FAILED) {
    push(trace, &filename);
    printsymbolmessage(ERRLVL_ERROR, trace, s->symbol, "Unexpected symbol!");
  }

  deleteArray(&trace);
  closeStream(s);
  deleteParser(&parser);
  deleteBNFTree(&bnftree);
  return ast;
}