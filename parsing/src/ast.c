#include <ast.h>

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
    new->status        = STATUS_NOSTATUS;
    new->continuations = 0;
    new->pos           = 0;
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
    if ((*node)->pos) (*node)->pos--;
    else {
      while ((*node)->subnodes->size) revertAST(pop((*node)->subnodes), s);
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

int isopening(Symbol *symbol, Parser *parser)
{
  for (int i = 0; parser->delimiters[i]; i++) {
    if (!strcmp(symbol->text, parser->delimiters[i])) {
      return (i + 1) % 2;
    }
  }
  return 0;
}

void astparsestream(ASTNode *ast, BNFNode *bnf, Array *rejected, ASTFlags flags, Stream *s)
{
  ASTNode  *subast;
  BNFNode  *subbnf;
  Symbol   *symbol  = s->symbol;
  int       size    = 0;
  char     *content = bnf->content;
  int       reclvl  = flags >> 8;

  if (bnf->type != NODE_LEAF && bnf->type != NODE_RAW) size = ((Array*)bnf->content)->size;
  if (!symbol->text) { 
    while (s->gets(s->stream) && (symbol->type == SYMBOL_COMMENT || symbol->type == SYMBOL_NEWLINE));
  }
  if (flags & ASTFLAGS_FRONT && bnf->refs->size > reclvl) {
    ASTNode *recnode = *(ASTNode**)last(bnf->refs);
    if (++recnode->pos == recnode->continuations) {
      ast->status = STATUS_PARTIAL;
      push(ast->subnodes, &recnode);
    } else ast->status = STATUS_FAILED;
    return;
  }
  flags = flags | (bnf->refs->size << 8);

  if (bnf->type != NODE_NOT) push(bnf->refs, &ast);
  do {
    ast->status = STATUS_NOSTATUS;
    switch (bnf->type) {
      case NODE_ROOT:
      case NODE_REC:
      case NODE_CONCAT:
        /// NOT APPLICABLE
        break;
      case NODE_NOT:
        if (!rejected) rejected = newArray(sizeof(BNFNode*));
        push(rejected, at(bnf->content, 1));
        astparsestream(ast, *(BNFNode**)at(bnf->content, 0), rejected, flags, s);
        pop(rejected);
        if (!rejected->size) deleteArray(&rejected);
        break;
      case NODE_RAW:
        if (symbol->type == (SymbolType)bnf->content && !(flags & ASTFLAGS_REC)) {
          int skip = symbol->type == SYMBOL_BREAK    || 
                     symbol->type == SYMBOL_OPERATOR || 
                     symbol->type == SYMBOL_RESERVED ||
                     isopening(symbol, s->parser);
          deleteString(&ast->name);
          deleteString(&ast->value);
          ast->name   = newString(bnf->name);
          ast->value  = newString(symbol->text);
          ast->symbol = newSymbol(symbol);
          ast->status = STATUS_CONFIRMED;
          while (s->gets(s->stream) && (symbol->type == SYMBOL_COMMENT || (skip && symbol->type == SYMBOL_NEWLINE)));
        } else ast->status = STATUS_FAILED;
        break;
      case NODE_LEAF:
        if (flags & ASTFLAGS_REC) ast->status = STATUS_FAILED;
        else if (content) {
          if (strcmp(symbol->text, content)) { ast->status = STATUS_FAILED; } 
          else {
            int skip = symbol->type == SYMBOL_BREAK    || 
                       symbol->type == SYMBOL_OPERATOR || 
                       symbol->type == SYMBOL_RESERVED ||
                       isopening(symbol, s->parser);
            deleteString(&ast->value);
            ast->value = newString(content);
            ast->symbol = newSymbol(symbol);
            ast->status = STATUS_CONFIRMED;
            while (s->gets(s->stream) && (symbol->type == SYMBOL_COMMENT || (skip && symbol->type == SYMBOL_NEWLINE)));
          }
        } else ast->status = STATUS_NULL;
        break;
      case NODE_LIST:
        for (int i = 0, f = flags & ~ASTFLAGS_RECLVL; i < size; i++) {
          subbnf = *(BNFNode**)at(bnf->content, i);
          subast = newASTNode(ast, subbnf);
          astparsestream(subast, subbnf, NULL, f, s);
          if (subast->status == STATUS_CONFIRMED) {
            f &= ~ASTFLAGS_FRONT;
          } else if (subast->status == STATUS_PARTIAL) {
            f &= ~ASTFLAGS_REC;
            f &= ~ASTFLAGS_FRONT;
          } else if (subast->status == STATUS_FAILED) {
            ast->status = STATUS_FAILED;
            while (ast->subnodes->size) revertAST(pop(ast->subnodes), s);
            if (!symbol->text) s->gets(s->stream);
            break;
          } else if (subast->status == STATUS_NULL) {
            deleteAST(pop(ast->subnodes));
          }
        }
        if (ast->status != STATUS_FAILED) ast->status = STATUS_CONFIRMED;
        if (flags & ASTFLAGS_REC && ast->status == STATUS_CONFIRMED) {
          subast = *(ASTNode**)at(ast->subnodes, 0);
          if (subast->status == STATUS_PARTIAL) {
            ASTNode *recnode = *(ASTNode**)pop(subast->subnodes);
            set(ast->subnodes, 0, rem(recnode->subnodes, 0));
            deleteAST(&subast);
          }
        }
        break;
      case NODE_ONE_OF:
      case NODE_ANON:
      case NODE_ONE_OR_NONE:
        subast = newASTNode(ast, NULL);
        for (int i = 0, f = bnf->name[0] ? flags | ASTFLAGS_FRONT : flags; i < size; i++) {
          subbnf = *(BNFNode**)at(bnf->content, i);
          if (rejected && in(rejected, &subbnf)) continue;
          astparsestream(subast, subbnf, rejected, f, s);
          if (subast->status == STATUS_CONFIRMED) {
            if (bnf->name[0] && bnf->type != NODE_ANON && !subast->name->length) {
              deleteString(&subast->name);
              subast->name = newString(bnf->name);
            }
            subast->ref = bnf; // Maybe to be deleted eventually
            ast->status = STATUS_CONFIRMED;
            break;
          } else if (subast->status == STATUS_NULL) {
            if (symbol->type == SYMBOL_EOF) { ast->status = STATUS_CONFIRMED; break; }
            else                            { ast->status = STATUS_NULL;             }
          }
        }
        if (ast->status != STATUS_CONFIRMED && ast->status != STATUS_NULL) {
          if (bnf->type == NODE_ONE_OR_NONE) ast->status = STATUS_NULL;
          else                               ast->status = STATUS_FAILED;
          deleteAST(pop(ast->subnodes));
        } else if (!ast->pos) {
          astupnode(ast, subast);
        }
        break;
      case NODE_MANY_OR_NONE:
      case NODE_MANY_OR_ONE:
        do {
          ast->status = STATUS_NOSTATUS;
          subast = newASTNode(ast, NULL);
          for (int i = 0; i < size; i++) {
            subbnf = *(BNFNode**)at(bnf->content, i);
            if (rejected && in(rejected, &subbnf)) continue;
            astparsestream(subast, subbnf, rejected, bnf->name[0] ? flags | ASTFLAGS_FRONT : flags, s);
            if (subast->status == STATUS_CONFIRMED) {
              ast->status = STATUS_CONFIRMED;
              break;
            } else if (subast->status == STATUS_NULL) {
              ast->status = STATUS_NULL;
            }
          }
          if (ast->status != STATUS_CONFIRMED && ast->status != STATUS_NULL) {
            ast->status = STATUS_FAILED;
            deleteAST(pop(ast->subnodes));
          }
        } while (ast->status == STATUS_CONFIRMED && symbol->type != SYMBOL_EOF);
        if (!ast->subnodes->size && bnf->type == NODE_MANY_OR_ONE) ast->status = STATUS_FAILED;
        /// TBD: Confirmed or null?
        else if (symbol->type != SYMBOL_EOF)                       ast->status = STATUS_CONFIRMED;
        break;
    }
    if (ast->continuations && !ast->pos && ast->subnodes->size == 1) {
      ast->status = STATUS_CONFIRMED;
      astupnode(ast, *(ASTNode**)last(ast->subnodes));
    }// else if (ast->continuations && ast->status == STATUS_FAILED && ast->subnodes->size == 1) {
    //   /// TEMPORARY: the first case should handle every case
    //   ast->status = STATUS_CONFIRMED;
    //   ast->pos = 0;
    //   astupnode(ast, *(ASTNode**)last(ast->subnodes));
    //   //printf("ok\n");
    // }
    ast->continuations = ast->pos;
    ast->pos = 0;
    flags |= ASTFLAGS_REC;
  } while (ast->status == STATUS_CONFIRMED && ast->continuations);
  if (bnf->type != NODE_NOT) pop(bnf->refs);
}

ASTNode *parseast(char *filename)
{
  BNFNode      *bnftree  = parsebnf("parsing/bnf/test.bnf");
  BNFNode      *rootent  = *(BNFNode**)at(bnftree->content, 0);
  Parser       *parser   = newParser("parsing/prs/csr.prs");
  Stream       *s        = getStreamSS(ssopen(filename, parser));
  ASTNode      *ast      = newASTNode(NULL, NULL);
  Array        *trace    = newArray(sizeof(char*));
  push(trace, &filename);

  astparsestream(ast, rootent, NULL, ASTFLAGS_NONE, s);
  if (s->symbol->type != SYMBOL_EOF || ast->status == STATUS_FAILED) {
    printsymbolmessage(ERRLVL_ERROR, trace, s->symbol, "Unexpected symbol!");
  }

  deleteArray(&trace);
  closeStream(s);
  deleteParser(&parser);
  deleteBNFTree(&bnftree);
  return ast;
}