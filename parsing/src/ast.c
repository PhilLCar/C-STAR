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
  ASTNode *supernode;
  int fail, success;
  char *content;
  switch (bnf->type) {
    case NODE_ROOT:
      return;
    case NODE_LEAF:
      if (c == AST_LOCK) {
        if (ast->status == STATUS_POTENTIAL) ast->status = STATUS_CONFIRMED;
        else ast->status = STATUS_FAILED;
      } else {
        content = bnf->content;
        if (content[ast->pos]) {
          if (content[ast->pos] == c) {
            if (!content[ast->pos + 1]) {
              ast->status = bnf->type == NODE_LEAF ? STATUS_POTENTIAL : STATUS_CONFIRMED;
            }
          } else {
            ast->status = STATUS_FAILED;
          }
        }
        ast->pos++;
      }
      break;
    case NODE_ONE_OR_NONE:
    case NODE_MANY_OR_NONE:
      if (!ast->subnodes->size) {
        subnode = astfrombnf(bnf);
        subnode->status = STATUS_ONGOING;
        pushastnode(ast, &subnode);
      } else {
        subnode = at(ast->subnodes, ast->subnodes->size - 1);
      }
      supernode = ast;
      ast = subnode;
    case NODE_LIST:
      success = 0;
      for (int i = 0; i < ((Array*)bnf->content)->size; i++) {
        subnode = at(ast->subnodes, i);
        if (!subnode) {
          subnode = astfrombnf(at(bnf->content, i));
          subnode->status = STATUS_ONGOING;
          pushastnode(ast, &subnode);
        }
        if (subnode->status != STATUS_CONFIRMED) {
          newcharast(subnode, at(bnf->content, i), c);
          if (subnode->status == STATUS_CONFIRMED) success++;
          break;
        } 
        if (subnode->status == STATUS_FAILED) {
          ast->status = STATUS_FAILED;
          break;
        } else if (subnode->status == STATUS_CONFIRMED) {
          success++;
        }
      }
      if (success == ((Array*)bnf->content)->size) {
        ast->status = STATUS_CONFIRMED;
      }

      if (bnf->type == NODE_ONE_OR_NONE || bnf->type == NODE_MANY_OR_NONE) {
        if (ast->status == STATUS_CONFIRMED) {
          if (bnf->type == NODE_ONE_OR_NONE) {
              supernode->status = STATUS_CONFIRMED;
          } else {
            ast = astfrombnf(bnf);
            ast->status = STATUS_ONGOING;
            pushastnode(supernode, &ast);
          }
        }
      }
      break;
    case NODE_ONE_OF:
      fail = 0;
      success = 0;
      for (int i = 0; i < ((Array*)bnf->content)->size; i++) {
        subnode = at(ast->subnodes, i);
        if (!subnode) {
          subnode = astfrombnf(at(bnf->content, i));
          subnode->status = STATUS_ONGOING;
          pushastnode(ast, &subnode);
        }
        if (subnode->status != STATUS_FAILED) {
          newcharast(subnode, at(bnf->content, i), c);
        }
        if (subnode->status == STATUS_FAILED) {
          fail++;
        } else if (subnode->status == STATUS_CONFIRMED) {
          ast->status = STATUS_CONFIRMED;
          success++;
        } 
      }
      if (success > 1) {
        // priority
      }
      if (fail == ast->subnodes->size) {
        ast->status = STATUS_FAILED;
      }
      break;
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