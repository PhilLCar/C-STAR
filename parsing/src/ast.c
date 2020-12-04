#include <ast.h>

#include <parser.h>
#include <strings.h>

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
    new->continuation  = 0;
    new->recurse       = 0;
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

void astnextsymbol(Stream *s) {
  while (s->gets(s->stream) && s->symbol->type == SYMBOL_COMMENT);
}

ASTStatus astparsestream(ASTNode *ast, BNFNode *bnf, Array *rejected, ASTFlags flags, Stream *s)
{
  BNFNode   *subbnf;
  ASTNode   *subast;
  ASTFlags   nflags;
  ASTStatus  status  = STATUS_FAILED;
  Symbol    *symbol  = s->symbol;
  int        size    = 0;
  char      *content = bnf->content;
  int        reclvl  = flags >> 8;

  if (rejected && in(rejected, &bnf)) {
    return status;
  }
  if (flags & ASTFLAGS_FRONT && bnf->refs->size > reclvl) {
    ASTNode *recnode = *(ASTNode**)last(bnf->refs);
    recnode->recurse = 1;
    if (recnode->continuation) {
      status = STATUS_PARTIAL;
      push(ast->subnodes, &recnode);
    }
    return status;
  }

  if (bnf->type != NODE_LEAF && bnf->type != NODE_RAW) size = ((Array*)bnf->content)->size;
  if (!symbol->text) astnextsymbol(s);

  flags = (flags & ~ASTFLAGS_RECLVL) | (bnf->refs->size << 8);
  if (bnf->type != NODE_NOT) push(bnf->refs, &ast);
  do {
    switch (bnf->type) {
    case NODE_ROOT:
    case NODE_REC:
    case NODE_CONCAT:
      /// NOT APPLICABLE (legacy)
      break;
    case NODE_NOT:
      if (!rejected) rejected = newArray(sizeof(BNFNode*));
      push(rejected, at(bnf->content, 1));
      status = astparsestream(ast, *(BNFNode**)at(bnf->content, 0), rejected, flags, s);
      pop(rejected);
      if (rejected && !rejected->size) deleteArray(&rejected);
      break;
    case NODE_RAW:
      if (!(flags & ASTFLAGS_REC) && symbol->type == (SymbolType)bnf->content) {
        deleteString(&ast->name);
        deleteString(&ast->value);
        ast->name   = newString(bnf->name);
        ast->value  = newString(symbol->text);
        ast->symbol = newSymbol(symbol);
        astnextsymbol(s);
        status = STATUS_CONFIRMED;
      }
      break;
    case NODE_LEAF:
      if (!(flags & ASTFLAGS_REC)) {
        if (content) {
          if (symbol->text && !strcmp(symbol->text, content)) {
            deleteString(&ast->value);
            ast->value  = newString(content);
            ast->symbol = newSymbol(symbol);
            astnextsymbol(s);
            status = STATUS_CONFIRMED;
          }
        } else status = STATUS_CONFIRMED;
      }
      break;
    case NODE_LIST:
      {
        int partial  = 0;
        nflags       = flags;
        for (int i = 0; i < size; i++) {
          subbnf = *(BNFNode**)at(bnf->content, i);
          if ((SymbolType)subbnf->content == SYMBOL_NO_NEWLINE) {
            subbnf = *(BNFNode**)at(bnf->content, ++i);
          } else if (i > 0) {
            // Ignore newlines when they would break the AST
            while (symbol->type == SYMBOL_NEWLINE) {
              subast = newASTNode(ast, NULL);
              deleteString(&subast->name);
              subast->name   = newString("<newline>");
              subast->symbol = newSymbol(symbol);
              astnextsymbol(s);
            }
          }
          subast = newASTNode(ast, subbnf);
          status = astparsestream(subast, subbnf, NULL, nflags, s);
          nflags &= ~ASTFLAGS_FRONT;
          if (status == STATUS_PARTIAL) {
            nflags &= ~ASTFLAGS_REC;
            partial = 1;
          } else if (status != STATUS_CONFIRMED) {
            while (ast->subnodes->size) revertAST(pop(ast->subnodes), s);
            if (!symbol->text) astnextsymbol(s);
            break;
          }
        }
        if (status != STATUS_CONFIRMED) status = STATUS_FAILED;
        // Can be optimized: returning status partial so NODE_ONE_OF doesn't have to check all other possibilities
        if (status == STATUS_CONFIRMED && partial) {
          ASTNode *recnode;
          subast  = *(ASTNode**)at(ast->subnodes, 0);
          recnode = *(ASTNode**)pop(subast->subnodes);
          set(ast->subnodes, 0, rem(recnode->subnodes, 0));
          deleteAST(&subast);
        }
      }
      break;
    case NODE_ONE_OF:
    case NODE_ANON:
    case NODE_ONE_OR_NONE:
      subast   = newASTNode(ast, NULL);
      nflags   = bnf->name[0] ? flags | ASTFLAGS_FRONT : flags;
      for (int i = 0; i < size; i++) {
        subbnf = *(BNFNode**)at(bnf->content, i);
        status = astparsestream(subast, subbnf, rejected, nflags, s);
        if (status == STATUS_CONFIRMED) {
          if (bnf->name[0] && bnf->type != NODE_ANON && !subast->name->length) {
            deleteString(&subast->name);
            subast->name = newString(bnf->name);
          }
          subast->ref = bnf; // Maybe to be deleted eventually
          break;
        }
      }
      if (status == STATUS_FAILED) {
        if (bnf->type == NODE_ONE_OR_NONE) status = STATUS_CONFIRMED;
        deleteAST(pop(ast->subnodes));
      } else if (!ast->recurse) {
        astupnode(ast, subast);
      }
      break;
    case NODE_MANY_OR_NONE:
    case NODE_MANY_OR_ONE:
      do {
        subast = newASTNode(ast, NULL);
        nflags = bnf->name[0] ? flags | ASTFLAGS_FRONT : flags;
        for (int i = 0; i < size; i++) {
          subbnf = *(BNFNode**)at(bnf->content, i);
          status = astparsestream(subast, subbnf, rejected, nflags, s);
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
    flags |= ASTFLAGS_REC;
  } while (status == STATUS_CONFIRMED && ast->continuation);
  if (bnf->type != NODE_NOT) pop(bnf->refs);

  return status;
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

  status = astparsestream(ast, rootent, NULL, ASTFLAGS_NONE, s);
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