#include <bnf_parser2.h>

BNFNode *parsebnfnode(char*, Parser*, BNFNode*, Array*);

BNFNode *getnode(BNFNode *basenode, BNFNode *node, char *name) {
  if (node == NULL || !strcmp(name, "")) return NULL;
  if (!strcmp(node->name, name)) return node;
  if (node == basenode) {
    basenode->rec++;
  } else if (node->rec < basenode-> rec) {
    node->rec = basenode->rec;
  } else {
    return NULL;
  }
  if (node->type != NODE_LEAF && node->type != NODE_LEAF_CONCAT) {
    Array *a = node->content;
    for (int i = 0; i < a->size; i++) {
      BNFNode *ret = getnode(basenode, at(a, i), name);
      if (ret) return ret;
    }
  }
  return NULL;
}

BNFNode *newBNFNode(BNFNode *basenode, char *name, BNFType type) {
  BNFNode *node = NULL;
  if (!basenode) {
    if (type != NODE_ROOT) {
      printmessage(ERROR, "A non-root node was created with no parent!");
      exit(1);
    }
  } else {
    node = getnode(basenode, basenode, name);
  }
  if (!node) {
    node     = malloc(sizeof(BNFNode));
    char  *n = malloc((strlen(name) + 1) * sizeof(char));
    Array *a = NULL;
    if (type != NODE_LEAF && type != NODE_LEAF_CONCAT) a = newArray(sizeof(BNFNode));
    if (node && n && (a || type == NODE_LEAF || type == NODE_LEAF_CONCAT)) {
      sprintf(n, "%s", name);
      node->name    = n;
      node->type    = type;
      node->content = a;
      node->rec     = 0;
    } else {
      if (node) free(node);
      if (n)    free(n);
      deleteArray(&a);
    }
  }
  return node;
}

void deleteBNFNode(BNFNode **node) {
  if (node) {
    free((*node)->name);
    if ((*node)->type == NODE_LEAF || (*node)->type == NODE_LEAF_CONCAT) {
      if ((*node)->content) free((*node)->content);
    } else {
      deleteArray((Array**)&(*node)->content);
    }
    *node = NULL;
  }
}

int expect(SymbolStream *ss, Array *trace, char *str) {
  char error[256];
  Symbol *s;

  if (!strcmp("\n", str)) {
    int junk = 0;
    Symbol *t = newSymbol(&ss->symbol);
    s = ssgets(ss);
    while (strcmp(s->text, str) && !s->eof) {
      s = ssgets(ss);
      junk = 1;
    }
    if (junk) {
      sprintf(error, "Junk after '"FONT_BOLD"%s"FONT_RESET"'", t->text);
      printsymbolmessage(ERROR, trace, t, error);
    }
    deleteSymbol(&t);
  }
  else {
    s = ssgets(ss);
    if (strcmp(s->text, str)) {
      sprintf(error, "Expected '"FONT_BOLD"%s"FONT_RESET"', got '"FONT_BOLD"%s"FONT_RESET"' instead!", str, s->text);
      printsymbolmessage(ERROR, trace, s, error);
      exit(1);
    }
  }

  return 1;
}

void parsebnfincludes(Parser *parser, SymbolStream *ss, BNFNode *basenode, Array *trace) {
  Symbol *s = NULL;
  if (trace->size > INCLUDE_MAX_DEPTH) {
    printfilemessage(ERROR, trace, "Maximum include depth reached, check for recursive includes");
    exit(1);
  }
  while ((s = ssgets(ss))) {
    if (strcmp(s->text, ";;") || s->eof) break;
    expect(ss, trace, "include");
    expect(ss, trace, "(");
    Symbol *n = newSymbol(&ss->symbol);
    String *a = newString("");
    while ((s = ssgets(ss))) {
      if (!strcmp(s->text, "\n")) {
        printsymbolmessage(ERROR, trace, n, "Expected closing parenthesis, reached newline instead");
        break;
      }
      if (s->eof) {
        printsymbolmessage(ERROR, trace, n, "Expected closing parenthesis, reached end of file instead");
        break;
      }
      if (!strcmp(s->text, ")")) {
        break;
      }
      concat(a, newString(s->text));
    }
    if (strcmp(a->content, "")) {
      parsebnfnode(a->content, parser, basenode, trace);
    } else {
      printsymbolmessage(ERROR, trace, n, "Expected file name inbetween parentheses");
    }
    deleteString(&a);
    deleteSymbol(&n);
    expect(ss, trace, "\n");
  }
  ssungets(ss, s);
}

int isoperator(Symbol *s) {
  if (!strcmp(s->text, "{")) return 1;
  if (!strcmp(s->text, "[")) return 1;
  if (!strcmp(s->text, "(")) return 1;
  if (!strcmp(s->text, "|")) return 1;
  if (!strcmp(s->text, "}")) return 1;
  if (!strcmp(s->text, "]")) return 1;
  if (!strcmp(s->text, ")")) return 1;
  return 0;
}

BNFNode *parsebnfnodename(SymbolStream *ss, BNFNode *basenode, Array *trace)
{
  Symbol *s = &ss->symbol;
  Symbol *t = newSymbol(s);
  s = ssgets(ss);
  if (s->eof) {
    printsymbolmessage(ERROR, trace, t, "Expected node name, reached end of file!");
  }
  if (!strcmp(s->text, "\n")) {
    printsymbolmessage(ERROR, trace, t, "Expected node name, reached new line!");
  }
  if (s->string || s->comment || isoperator(s)) {
    printsymbolmessage(ERROR, trace, s, "Bad format for node name!");
  }
  if (!strcmp(s->text, basenode->name)) {
    printsymbolmessage(ERROR, trace, s, "Cannot use that name! (Reserved for root node)");
  }
  deleteSymbol(&t);
  return newBNFNode(basenode, s->text, NODE_ONE_OF);
}

int parsebnfstatement(SymbolStream *ss, BNFNode *basenode, BNFNode *parent, Array *trace, char *stop)
{
  int oplast, ret = 1;
  Symbol *s;
  BNFNode *subnode = newBNFNode(basenode, "", NODE_LIST);
  push(parent->content, subnode);

  do {
    oplast = 0;
    s = ssgets(ss);
    if (s->eof) return 0;
    if (s->comment) continue;
    if (!strcmp(s->text, "(")) {
      BNFNode *node = newBNFNode(basenode, "", NODE_ONE_OF);
      push(subnode->content, node);
      ret = parsebnfstatement(ss, basenode, node, trace, ")");
    }
    if (!strcmp(s->text, "[")) {
      BNFNode *node = newBNFNode(basenode, "", NODE_ONE_OR_NONE);
      push(subnode->content, node);
      ret = parsebnfstatement(ss, basenode, node, trace, "]");
    }
    if (!strcmp(s->text, "{")) {
      BNFNode *node = newBNFNode(basenode, "", NODE_MANY_OR_NONE);
      push(subnode->content, node);
      ret = parsebnfstatement(ss, basenode, node, trace, "}");
    }
    if (!strcmp(s->text, "|")) {
      subnode = newBNFNode(basenode, "", NODE_LIST);
      push(parent->content, subnode);
      oplast = 1;
    }
    if (!strcmp(s->text, "<")) {
      push(subnode->content, parsebnfnodename(ss, basenode, trace));
      expect(ss, trace, ">");
    }
    if (s->string) {
      BNFNode *node = NULL;
      char *content = malloc((strlen(s->text) + 1) * sizeof(char));
      sprintf(content, "%s", s->text);
      if      (!strcmp(s->open, "\"")) node = newBNFNode(basenode, "", NODE_LEAF);
      else if (!strcmp(s->open, "'"))  node = newBNFNode(basenode, "", NODE_LEAF_CONCAT);
      node->content = content;
      push(subnode->content, node);
    }
  } while(ret && (strcmp(s->text, stop) || (stop[0] == '\n' && oplast)));
  return ret;
}

void parsebnfbody(SymbolStream *ss, BNFNode *basenode, Array *trace)
{
  Symbol *s;
  while ((s = ssgets(ss))) {
    if (s->eof) break;
    if (!strcmp(s->text, "\n")) continue;
    if (strcmp(s->text, "<")) {
      char error[256];
      sprintf(error, "Expected '"FONT_BOLD"<"FONT_RESET"', got '"FONT_BOLD"%s"FONT_RESET"' instead!", s->text);
      printsymbolmessage(ERROR, trace, s, error);
      break;
    } else {
      BNFNode *node = parsebnfnodename(ss, basenode, trace);
      expect(ss, trace, ">");
      expect(ss, trace, "::=");
      push(basenode->content, node);
      if (!parsebnfstatement(ss, basenode, node, trace, "\n")) break;
    }
  }
}

BNFNode *parsebnfnode(char *filename, Parser *parser, BNFNode *basenode, Array *trace)
{
  SymbolStream *ss = ssopen(filename, parser);
  push(trace, &filename);

  if (!ss) {
    printfilemessage(ERROR, trace, "Could not open file!");
    exit(1);
  }

  parsebnfincludes(parser, ss, basenode, trace);
  parsebnfbody(ss, basenode, trace);

  pop(trace);
  ssclose(ss);
  return basenode;
}


BNFNode *parsebnf(char *filename) 
{
  // TODO: Have parser saved to file for quicker recuperation
  Parser  *parser   = newParser("parsing/prs/bnf.prs");
  BNFNode *basenode;
  Array   *trace    = newArray(sizeof(char*));
  char     nodename[256];

  sprintf(nodename, "root:%s", filename);
  basenode = newBNFNode(NULL, nodename, NODE_ROOT);
  parsebnfnode(filename, parser, basenode, trace);
  
  deleteParser(&parser);
  return basenode;
}

void deleteBNFTree(BNFNode *basenode, BNFNode *node)
{
  if (node == NULL) return;
  if (node == basenode) {
    basenode->rec++;
  } else if (node->rec < basenode-> rec) {
    node->rec = basenode->rec;
  } else {
    return;
  }
  if (node->type != NODE_LEAF && node->type != NODE_LEAF_CONCAT) {
    Array *a = node->content;
    for (int i = 0; i < a->size; i++) {
      deleteBNFTree(basenode, at(a, i));
    }
  }
  deleteBNFNode(&node);
}