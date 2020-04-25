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
    if (bnf->type == NODE_LEAF) {
      ast->name = newString(bnf->content);
    } else {
      ast->name = newString(bnf->name);
    }
    ast->subnodes = newArray(sizeof(ASTNode));
    ast->value    = newString("");
    ast->status   = STATUS_NOSTATUS;
    ast->ref      = bnf;
    ast->pos      = 0;
  }
  return ast;
}

void newcharast(ASTNode *ast, BNFNode *bnf, char c)
{
  ASTNode *subnode;
  BNFNode *subbnf;
  int n;
  char *content;
  switch (bnf->type) {
    case NODE_ROOT:
      return;
    case NODE_LEAF:
    case NODE_LEAF_CONCAT:
      content = bnf->content;
      if (content[ast->pos]) {
        if (content[ast->pos] == c) {
          if (content[ast->pos + 1]) {
            ast->status = STATUS_ONGOING;
          } else {
            ast->status = bnf->type == NODE_LEAF ? STATUS_POTENTIAL : STATUS_CONFIRMED;
          }
        } else {
          ast->status = STATUS_FAILED;
        }
      }
      ast->pos++;
      return;
    case NODE_ONE_OR_NONE:
    case NODE_MANY_OR_NONE:
      if (!ast->subnodes->size) {
        subnode = astfrombnf(bnf);
        subnode->status = STATUS_NOSTATUS;
        pushastnode(ast, &subnode);
      } else {
        subnode = at(ast->subnodes, ast->subnodes->size - 1);
      }
      if (subnode->status == STATUS_CONFIRMED) {
        if (bnf->type == NODE_ONE_OR_NONE) {
            ast->status = STATUS_CONFIRMED;
            return;
        } else {
          subnode = astfrombnf(bnf);
          subnode->status = STATUS_NOSTATUS;
          pushastnode(ast, &subnode);
        }
      }
      ast = subnode;
    case NODE_LIST:
      n = 0;
      for (int i = 0; i < ast->subnodes->size; i++) {
        subnode = at(ast->subnodes, i);
        if (subnode->status == STATUS_ONGOING) {
          newcharast(subnode, at(bnf->content, i), c);
        } else if (subnode->status == STATUS_FAILED) {
          ast->status = STATUS_FAILED;
          return;
        } else if (subnode->status == STATUS_CONFIRMED) {
          n++;
        }
      }
      if (n == ((Array*)bnf->content)->size) {
        ast->status = STATUS_CONFIRMED;
        return;
      }
      ast->status = STATUS_ONGOING;
      subbnf = at(bnf->content, ast->subnodes->size);
      subnode = astfrombnf(subbnf);
      subnode->status = STATUS_ONGOING;
      pushastnode(ast, &subnode);
      newcharast(subnode, subbnf, c);
      return;
    case NODE_ONE_OF:
      n = 0;
      for (int i = 0; i < ((Array*)bnf->content)->size; i++) {
        subnode = at(ast->subnodes, i);
        if (!subnode) {
          ast->status = STATUS_ONGOING;
          subnode = astfrombnf(at(bnf->content, i));
          subnode->status = STATUS_ONGOING;
          pushastnode(ast, &subnode);
        }
      }
      for (int i = 0; i < ast->subnodes->size; i++) {
        subnode = at(ast->subnodes, i);
        if (subnode->status == STATUS_CONFIRMED) {
          ast->status = STATUS_CONFIRMED;
          break;
        } else  if (subnode->status == STATUS_FAILED) {
          n++;
        } else {
          newcharast(subnode, at(bnf->content, i), c);
        }
      }
      if (n == ((Array*)bnf->content)->size) {
        ast->status = STATUS_FAILED;
        return;
      }
      return;
  }
  return;
}


ASTNode *parseast(char *filename)
{
  BNFNode      *root   = parsebnf("parsing/bnf/test.bnf");
  Parser       *parser = newParser("parsing/prs/csr.prs");
  SymbolStream *ss     = ssopen(filename, parser);
  //ASTNode      *ast    = initast(root);

  Symbol *s;
  while (!(s = ssgets(ss))->eof) {
    printf("%s\n", s->text);
  }

  ssclose(ss);
  deleteParser(&parser);
  deleteBNFTree(&root);
  return NULL;
}