#include <bnf.h>

BNFNode *parsebnfnode(char*, Parser*, BNFNode*, Array*, Array*);

void checkbnfnode(BNFNode *node)
{
  if (!node) {
    printmessage(ERRLVL_ERROR, "Memory allocation failed!");
    exit(1);
  }
}

BNFNode *getnode(BNFNode *basenode, BNFNode *node, char *name)
{
  if (node == NULL || !strcmp(name, "")) return NULL;
  if (!strcmp(node->name, name)) return node;
  if (node == basenode) {
    basenode->rec++;
  } else if (node->rec < basenode->rec) {
    node->rec = basenode->rec;
  } else {
    return NULL;
  }
  if (node->type != NODE_LEAF) {
    Array *a = node->content;
    for (int i = 0; i < a->size; i++) {
      BNFNode *ret = getnode(basenode, *(BNFNode**)at(a, i), name);
      if (ret) return ret;
    }
  }
  return NULL;
}

BNFNode *newBNFNode(BNFNode *basenode, char *name, BNFType type)
{
  BNFNode *node = NULL;
  if (!basenode) {
    if (type != NODE_ROOT) {
      printmessage(ERRLVL_ERROR, "A non-root node was created with no parent!");
      exit(1);
    }
  } else if (getnode(basenode, basenode, name)) {
    printmessage(ERRLVL_ERROR, "Cannot add two nodes of the same name to the tree!");
  }
  node     = malloc(sizeof(BNFNode));
  char  *n = malloc((strlen(name) + 1) * sizeof(char));
  Array *a = NULL;
  if (type != NODE_LEAF) a = newArray(sizeof(BNFNode*));
  if (node && n && (a || type == NODE_LEAF)) {
    sprintf(n, "%s", name);
    node->name    = n;
    node->type    = type;
    node->content = a;
    node->rec     = 0;
    node->refs    = newArray(sizeof(void*));
  } else {
    if (node) free(node);
    if (n)    free(n);
    deleteArray(&a);
  }
  return node;
}

void deleteBNFNode(BNFNode **node)
{
  if (*node) {
    free((*node)->name);
    if ((*node)->type == NODE_LEAF) {
      if ((*node)->content) free((*node)->content);
    } else {
      deleteArray((Array**)&(*node)->content);
    }
    deleteArray(&(*node)->refs);
    free(*node);
  }
}

int expect(SymbolStream *ss, Array *trace, char *str)
{
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
      printsymbolmessage(ERRLVL_ERROR, trace, t, error);
    }
    deleteSymbol(&t);
  }
  else {
    s = ssgets(ss);
    if (strcmp(s->text, str)) {
      sprintf(error, "Expected '"FONT_BOLD"%s"FONT_RESET"', got '"FONT_BOLD"%s"FONT_RESET"' instead!", str, s->text);
      printsymbolmessage(ERRLVL_ERROR, trace, s, error);
      exit(1);
    }
  }

  return 1;
}

int isoperator(Symbol *s) {
  if (!strcmp(s->text, "{")) return 1;
  if (!strcmp(s->text, "[")) return 1;
  if (!strcmp(s->text, "(")) return 1;
  if (!strcmp(s->text, "|")) return 1;
  if (!strcmp(s->text, "}")) return 1;
  if (!strcmp(s->text, "]")) return 1;
  if (!strcmp(s->text, ")")) return 1;
  if (!strcmp(s->text, ",")) return 1;
  return 0;
}

char *parsebnfname(SymbolStream *ss, BNFNode *basenode, Array *trace)
{
  Symbol *s = &ss->symbol;
  Symbol *t = newSymbol(s);

  s = ssgets(ss);
  if (s->eof) {
    printsymbolmessage(ERRLVL_ERROR, trace, t, "Expected node name, reached end of file!");
    exit(1);
  }
  if (!strcmp(s->text, "\n")) {
    printsymbolmessage(ERRLVL_ERROR, trace, t, "Expected node name, reached new line!");
    exit(1);
  }
  if (s->string || s->comment || isoperator(s)) {
    printsymbolmessage(ERRLVL_ERROR, trace, s, "Bad format for node name!");
    exit(1);
  }
  if (!strcmp(s->text, basenode->name)) {
    printsymbolmessage(ERRLVL_ERROR, trace, s, "Cannot use that name! (Reserved for root node)");
    exit(1);
  }
  deleteSymbol(&t);

  return s->text;
}

int parsebnfstatement(SymbolStream *ss, BNFNode *basenode, BNFNode *parent, Array *trace, char *stop)
{
  int oplast = 0, ret = 1, concat = 0;
  Symbol  *s;
  BNFNode *subnode = newBNFNode(basenode, "", NODE_LIST);
  Array   *content;
  push(parent->content, &subnode);

  do {
    s = ssgets(ss);
    if (s->eof) {
      ret = 0;
      break;
    }
    if (s->comment) continue;
    if (concat) {
      content = (*(BNFNode**)at(content, content->size - 1))->content;
      concat = 0;
    } else {
      content = subnode->content;
    }
    if (s->text[0] == '(') {
      BNFNode *node = newBNFNode(basenode, "", NODE_ONE_OF);
      checkbnfnode(node);
      push(content, &node);
      ret = parsebnfstatement(ss, basenode, node, trace, ")");
      oplast = 0;
    }
    else if (s->text[0] == '[') {
      BNFNode *node = newBNFNode(basenode, "", NODE_ONE_OR_NONE);
      checkbnfnode(node);
      push(content, &node);
      ret = parsebnfstatement(ss, basenode, node, trace, "]");
      if (EBNF_TO_BNF) {
        BNFNode *empty = newBNFNode(basenode, "", NODE_LEAF);
        node->type = NODE_ONE_OF;
        push(node->content, &empty);
      }
      oplast = 0;
    }
    else if (s->text[0] == '{') {
      char name[32] = "";
      if (EBNF_TO_BNF) sprintf(name, REC_NODE_INDICATOR"%d", basenode->rec);
      BNFNode *node = newBNFNode(basenode, name, EBNF_TO_BNF ? NODE_REC : NODE_MANY_OR_NONE);
      checkbnfnode(node);
      push(content, &node);
      ret = parsebnfstatement(ss, basenode, node, trace, "}");
      if (EBNF_TO_BNF) {
        BNFNode *list  = newBNFNode(basenode, "", NODE_LIST);
        BNFNode *empty = newBNFNode(basenode, "", NODE_LEAF);
        BNFNode *n     = *(BNFNode**)pop(node->content);
        push(list->content, &n);
        push(list->content, &node);
        push(node->content, &list);
        push(node->content, &empty);
      }
      Symbol *t = newSymbol(s);
      s = ssgets(ss);
      if (s->text[0] == '+') {
        if (EBNF_TO_BNF) {
          BNFNode *n = *(BNFNode**)pop(content);
          BNFNode *l = newBNFNode(basenode, "", NODE_LIST);
          push(l->content, at((*(BNFNode**)at(node->content, 0))->content, 0));
          push(l->content, &n);
          push(content, &l);
        } else {
          node->type = NODE_MANY_OR_ONE;
        }
      } else {
        ssungets(ss, s);
        ssungets(ss, t);
        s = ssgets(ss);
        deleteSymbol(&t);
      }
      oplast = 0;
    }
    else if (s->text[0] == '|') {
      if (!content->size) {
        printsymbolmessage(ERRLVL_WARNING, trace, s, "Empty group ignored");
      } else if (content != subnode->content) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected operator!");
        ret = 0;
      } else {
        if (content->size == 1) {
          subnode->content = NULL;
          deleteBNFNode(pop(parent->content));
          push(parent->content, pop(content));
          deleteArray(&content);
        }
        subnode = newBNFNode(basenode, "", NODE_LIST);
        checkbnfnode(subnode);
        push(parent->content, &subnode);
        content = subnode->content;
      }
      oplast = 1;
    }
    else if (s->text[0] == ',') {
      if (!content->size) {
        printsymbolmessage(ERRLVL_WARNING, trace, s, "Empty group ignored");
      } else if (content != subnode->content) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected operator!");
        ret = 0;
      } else {
        BNFNode *node = newBNFNode(basenode, "", NODE_CONCAT);
        checkbnfnode(node);
        push(node->content, pop(subnode->content));
        push(subnode->content, &node);
        concat = 1;
      }
      oplast = 1;
    }
    else if (s->text[0] == '<') {
      char *name =  parsebnfname(ss, basenode, trace);
      BNFNode *node = getnode(basenode, basenode, name);
      if (!node) {
        node = newBNFNode(basenode, name, NODE_ONE_OF);
      }
      checkbnfnode(node);
      push(content, &node);
      expect(ss, trace, ">");
      oplast = 0;
    }
    else if (s->string) {
      BNFNode *node = NULL;
      char *value = s->text[0] ? malloc((strlen(s->text) + 1) * sizeof(char)) : NULL;
      if (value) sprintf(value, "%s", s->text);
      node = newBNFNode(basenode, "", NODE_LEAF);
      if (!strcmp(s->open, "\"")) {
        if (strcmp(s->close, "\"")) printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected closing '"FONT_BOLD"\""FONT_RESET"'!");
      } else if (!strcmp(s->open, "'")) {
        if (strcmp(s->close, "\'")) printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected closing '"FONT_BOLD"'"FONT_RESET"'!");
      }
      checkbnfnode(node);
      node->content = value;
      push(content, &node);
      oplast = 0;
    }
    else if (!strcmp(s->text, stop)) {
      continue;
    }
    else {
      char error[256];
      sprintf(error, "Unexpected symbol '"FONT_BOLD"%s"FONT_RESET"'!", s->text);
      printsymbolmessage(ERRLVL_ERROR, trace, s, error);
      ret = 0;
    }
  } while(ret && (strcmp(s->text, stop) || (stop[0] == '\n' && oplast)));

  if (!content->size) {
    printsymbolmessage(ERRLVL_WARNING, trace, s, "Empty group ignored");
    deleteBNFNode(pop(parent->content));
  } else if (content->size == 1) {
    subnode->content = NULL;
    deleteBNFNode(pop(parent->content));
    push(parent->content, pop(content));
    deleteArray(&content);
  }
  return ret;
}

void parsebnfincludes(Parser *parser, SymbolStream *ss, BNFNode *basenode, Array *trace, Array *includes) {
  Symbol *s = NULL;
  if (trace->size > INCLUDE_MAX_DEPTH) {
    printfilemessage(ERRLVL_ERROR, trace, "Maximum include depth reached, check for recursive includes");
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
        printsymbolmessage(ERRLVL_ERROR, trace, n, "Expected closing parenthesis, reached newline instead");
        break;
      }
      if (s->eof) {
        printsymbolmessage(ERRLVL_ERROR, trace, n, "Expected closing parenthesis, reached end of file instead");
        break;
      }
      if (!strcmp(s->text, ")")) {
        break;
      }
      concat(a, newString(s->text));
    }
    if (strcmp(a->content, "")) {
      int parse = 1;
      for (int i = 0; i < includes->size; i++) {
        if (!strcmp(a->content, ((String*)at(includes, i))->content)) {
          parse = 0;
          break;
        }
      }
      if (parse) parsebnfnode(a->content, parser, basenode, trace, includes);
      String *str = newString(a->content);
      pushobj(includes, str);
    } else {
      printsymbolmessage(ERRLVL_ERROR, trace, n, "Expected file name inbetween parentheses");
    }
    deleteString(&a);
    deleteSymbol(&n);
    expect(ss, trace, "\n");
  }
  ssungets(ss, s);
}

void parsebnfbody(SymbolStream *ss, BNFNode *basenode, Array *trace)
{
  Symbol *s;
  while ((s = ssgets(ss))) {
    if (s->eof) break;
    if (!strcmp(s->text, "\n")) continue;
    if (s->comment)             continue;
    if (strcmp(s->text, "<")) {
      char error[256];
      sprintf(error, "Expected '"FONT_BOLD"<"FONT_RESET"', got '"FONT_BOLD"%s"FONT_RESET"' instead!", s->text);
      printsymbolmessage(ERRLVL_ERROR, trace, s, error);
      break;
    } else {
      char *name =  parsebnfname(ss, basenode, trace);
      BNFNode *node = getnode(basenode, basenode, name);
      if (!node) {
        node = newBNFNode(basenode, name, NODE_ONE_OF);
      }
      checkbnfnode(node);
      expect(ss, trace, ">");
      expect(ss, trace, "::=");
      push(basenode->content, &node);
      if (!parsebnfstatement(ss, basenode, node, trace, "\n")) break;
    }
  }
}

BNFNode *parsebnfnode(char *filename, Parser *parser, BNFNode *basenode, Array *trace, Array *includes)
{
  SymbolStream *ss = ssopen(filename, parser);
  push(trace, &filename);

  if (!ss) {
    printfilemessage(ERRLVL_ERROR, trace, "Could not open file!");
    exit(1);
  }

  parsebnfincludes(parser, ss, basenode, trace, includes);
  parsebnfbody(ss, basenode, trace);

  pop(trace);
  ssclose(ss);
  return basenode;
}

void linkbnf(BNFNode *basenode, Array* trace) {
  Array *a = basenode->content;
  BNFNode *n;
  for (int i = 0; i < a->size;) {
    n = *(BNFNode**)rem(a, 0);
    if (n != NULL && getnode(basenode, basenode, n->name) == NULL) {
      push(a, &n);
      i++;
    }
  }
  if (a->size == 0) {
    printnodemessage(ERRLVL_WARNING, trace, basenode->name, "Node is empty!");
  } else if (a->size > 1) {
    printnodemessage(ERRLVL_WARNING, trace, basenode->name, "Multiple roots!");
    for (int i = 0; i < a->size; i++) {
      BNFNode *root = *(BNFNode**)at(a, i);
      fprintf(stderr, "    :<%s>\n", root->name);
    }
  }
}

BNFNode *parsebnf(char *filename) 
{
  // TODO: Have parser saved to file for quicker recuperation
  Parser  *parser   = newParser("parsing/prs/bnf.prs");
  BNFNode *basenode;
  Array   *trace    = newArray(sizeof(char*));
  Array   *includes = newArray(sizeof(String));
  char     nodename[256];

  pushobj(includes, newString(filename));
  sprintf(nodename, "root:%s", filename);
  basenode = newBNFNode(NULL, nodename, NODE_ROOT);
  parsebnfnode(filename, parser, basenode, trace, includes);
  push(trace, &filename);
  linkbnf(basenode, trace);
  
  deleteParser(&parser);
  deleteArray(&trace);
  while (popobj(includes, (F)freestring));
  deleteArray(&includes);
  return basenode;
}

void bnfpushunique(Array *a, BNFNode *n) {
  for (int i = 0; i < a->size; i++) {
    if ((*(BNFNode**)at(a, i)) == n) return;
  }
  push(a, &n);
}

void delbnftree(BNFNode *basenode, BNFNode *node, Array *unique)
{
  if (node == NULL) return;
  if (node == basenode) {
    basenode->rec++;
  } else if (node->rec < basenode->rec) {
    node->rec = basenode->rec;
  } else {
    return;
  }
  if (node->type != NODE_LEAF) {
    Array *a = node->content;
    for (int i = 0; i < a->size; i++) {
      delbnftree(basenode, *(BNFNode**)at(a, i), unique);
    }
  }
  bnfpushunique(unique, node);
}

void deleteBNFTree(BNFNode **basenode)
{
  if (*basenode) {
    Array *unique = newArray(sizeof(BNFNode*));
    delbnftree(*basenode, *basenode, unique);
    while (unique->size) deleteBNFNode(pop(unique));
    deleteArray(&unique);
    *basenode = NULL;
  }
}