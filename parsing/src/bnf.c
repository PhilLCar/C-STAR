#include <bnf.h>

BNFNode *parsebnfnode(char*, Parser*, BNFNode*, Array*, Array*);

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
  if (node->type != NODE_LEAF && node->type != NODE_RAW) {
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
      return NULL;
    }
  } else if (getnode(basenode, basenode, name)) {
    printmessage(ERRLVL_ERROR, "Cannot add two nodes of the same name to the tree!");
  }
  node     = malloc(sizeof(BNFNode));
  char  *n = malloc((strlen(name) + 1) * sizeof(char));
  Array *a = NULL;
  if (type != NODE_LEAF && type != NODE_RAW) a = newArray(sizeof(BNFNode*));
  if (node && n && (a || type == NODE_LEAF || type == NODE_RAW)) {
    sprintf(n, "%s", name);
    node->name    = n;
    node->def     = BNF_NOT_DEFINED;
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
    } else if ((*node)->type != NODE_RAW) {
      deleteArray((Array**)&(*node)->content);
    }
    deleteArray(&(*node)->refs);
    free(*node);
  }
}

int expect(SymbolStream *ss, Array *trace, char *str)
{
  int ok = 1;
  char error[256];
  Symbol *s;

  if (!strcmp("\n", str)) {
    int junk = 0;
    Symbol *t = newSymbol(&ss->symbol);
    s = ssgets(ss);
    while (s->type != SYMBOL_NEWLINE && s->type != SYMBOL_EOF) {
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
      ok = 0;
    }
  }

  return ok;
}

BNFNode *parsebnfname(SymbolStream *ss, BNFNode *basenode, Array *trace, BNFDef def)
{
  BNFNode *node = NULL;
  Symbol *s     = &ss->symbol;
  Symbol *t     = newSymbol(s);

  s = ssgets(ss);
  if (s->type == SYMBOL_EOF) {
    printsymbolmessage(ERRLVL_ERROR, trace, t, "Expected node name, reached end of file!");
  } else if (s->type == SYMBOL_NEWLINE) {
    printsymbolmessage(ERRLVL_ERROR, trace, t, "Expected node name, reached new line!");
  } else if (s->type == SYMBOL_STRING || s->type == SYMBOL_COMMENT || s->type == SYMBOL_OPERATOR) {
    printsymbolmessage(ERRLVL_ERROR, trace, s, "Bad format for node name!");
  } else if (!strcmp(s->text, "root")) {
    printsymbolmessage(ERRLVL_ERROR, trace, s, "Cannot use that name! (Reserved for root node)");
  } else if (!strcmp(s->text, "raw")) {
    if (expect(ss, trace, ":")) {
      char name[32];
      s = ssgets(ss);
      if (s->type == SYMBOL_RESERVED) {
        sprintf(name, "<%s>", s->text);
        node = getnode(basenode, basenode, name);
        if (!node) {
          node = newBNFNode(basenode, name, NODE_RAW);
          if (!strcmp(s->text, "integer")) {
            node->content = (void*)SYMBOL_INTEGER;
          } else if (!strcmp(s->text, "decimal")) {
            node->content = (void*)SYMBOL_DECIMAL;
          } else if (!strcmp(s->text, "string")) {
            node->content = (void*)SYMBOL_STRING;
          } else if (!strcmp(s->text, "variable")) {
            node->content = (void*)SYMBOL_VARIABLE;
          } else if (!strcmp(s->text, "char")) {
            node->content = (void*)SYMBOL_CHAR;
          } else if (!strcmp(s->text, "newline")) {
            node->content = (void*)SYMBOL_NEWLINE;
          } else if (!strcmp(s->text, "constant")) {
            node->content = (void*)SYMBOL_CONSTANT;
          } else if (!strcmp(s->text, "new-type")) {
            node->content = NULL;
          } else if (!strcmp(s->text, "no-newline")) {
            node->content = (void*)SYMBOL_NO_NEWLINE;
          }
        }
      } else {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Invalid raw type!");
      }
    }
  } else if (!strcmp(s->text, "forward")) {
    if (expect(ss, trace, ":")) {
      s = ssgets(ss);
      if (s->type == SYMBOL_VARIABLE) {
        node = getnode(basenode, basenode, s->text);
        if (!node) {
          node = newBNFNode(basenode, s->text, NODE_ONE_OF);
          node->def = BNF_DECLARED;
        } else node->def = BNF_DEFINED_AND_DECLARED;
      } else {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Invalid forward node!");
      }
    }
  } else if (!strcmp(s->text, "anon")) {
    if (expect(ss, trace, ":")) {
      s = ssgets(ss);
      if (s->type == SYMBOL_VARIABLE) {
        node = getnode(basenode, basenode, s->text);
        if (!node) {
          node = newBNFNode(basenode, s->text, NODE_ANON);
        }
      } else {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Invalid anonymous node!");
      }
    }
  } else {
    node = getnode(basenode, basenode, s->text);
    if (!node) {
      node = newBNFNode(basenode, s->text, NODE_ONE_OF);
    }
    if (def == BNF_DEFINED) node->def = def;
  }
  deleteSymbol(&t);

  return node;
}

void bnfsimplify(BNFNode *parent, BNFNode *subnode, Array *content)
{
  BNFNode *child = *(BNFNode**)at(content, 0);
  if (child->type != NODE_REC) {
    subnode->content = NULL;
    deleteBNFNode(pop(parent->content));
    push(parent->content, pop(content));
    deleteArray(&content);
  }
}

int parsebnfstatement(SymbolStream *ss, BNFNode *basenode, BNFNode *parent, Array *trace, char *stop)
{
  int oplast = 0, ret = 1, concat = 0;
  Symbol  *s;
  BNFNode *subnode = newBNFNode(basenode, "", NODE_LIST);
  Array   *content = NULL;
  push(parent->content, &subnode);

  do {
    s = ssgets(ss);
    if (s->type == SYMBOL_EOF) {
      ret = 0;
      break;
    }
    if (s->type == SYMBOL_COMMENT) continue;
    if (concat) {
      content = (*(BNFNode**)at(content, content->size - 1))->content;
      concat = 0;
    } else {
      content = subnode->content;
    } //////////////////////////////////////////////////////////////////////////////////////////
    if (s->type == SYMBOL_STRING) {
      char a, b;
      for (int i = 0; (a = ss->parser->whitespaces[i]); i++) {
        for (int j = 0; (b = s->text[j]); j++) {
          if (a == b) {
            printsymbolmessage(ERRLVL_ERROR, trace, s, "A leaf element cannot contain a whitespace!");
            ret = 0;
            break;
          }
        }
      }
      BNFNode *node = NULL;
      char *value = s->text[0] ? malloc((strlen(s->text) + 1) * sizeof(char)) : NULL;
      if (value) sprintf(value, "%s", s->text);
      node = newBNFNode(basenode, "", NODE_LEAF);
      if (!strcmp(s->open, "\"")) {
        if (strcmp(s->close, "\"")) printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected closing '"FONT_BOLD"\""FONT_RESET"'!");
      } else if (!strcmp(s->open, "'")) {
        if (strcmp(s->close, "\'")) printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected closing '"FONT_BOLD"'"FONT_RESET"'!");
      }
      node->content = value;
      push(content, &node);
      oplast = 0;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->text[0] == '(') {
      BNFNode *node = newBNFNode(basenode, "", NODE_ONE_OF);
      push(content, &node);
      ret = parsebnfstatement(ss, basenode, node, trace, ")");
      oplast = 0;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->text[0] == '[') {
      BNFNode *node = newBNFNode(basenode, "", NODE_ONE_OR_NONE);
      push(content, &node);
      ret = parsebnfstatement(ss, basenode, node, trace, "]");
      if (EBNF_TO_BNF) {
        BNFNode *empty = newBNFNode(basenode, "", NODE_LEAF);
        node->type = NODE_ONE_OF;
        push(node->content, &empty);
      }
      oplast = 0;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->text[0] == '{') {
      char name[32] = "";
      if (EBNF_TO_BNF) sprintf(name, "ID: %d", basenode->rec);
      BNFNode *node = newBNFNode(basenode, name, EBNF_TO_BNF ? NODE_REC : NODE_MANY_OR_NONE);
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
        ssungets(s, ss);
        ssungets(t, ss);
        s = ssgets(ss);
      }
      deleteSymbol(&t);
      oplast = 0;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->text[0] == '|') {
      if (!content->size) {
        printsymbolmessage(ERRLVL_WARNING, trace, s, "Empty group ignored");
      } else if (content != subnode->content) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected operator!");
        ret = 0;
      } else {
        if (content->size == 1) {
          bnfsimplify(parent, subnode, content);
        }
        subnode = newBNFNode(basenode, "", NODE_LIST);
        push(parent->content, &subnode);
        content = subnode->content;
      }
      oplast = 1;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->text[0] == ',') {
      if (!content->size) {
        printsymbolmessage(ERRLVL_WARNING, trace, s, "Empty group ignored");
      } else if (content != subnode->content) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected operator!");
        ret = 0;
      } else {
        BNFNode *node = newBNFNode(basenode, "", NODE_CONCAT);
        push(node->content, pop(subnode->content));
        push(subnode->content, &node);
        concat = 1;
      }
      oplast = 1;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->text[0] == '-') {
      if (!content->size) {
        printsymbolmessage(ERRLVL_WARNING, trace, s, "Empty group ignored");
      } else if (content != subnode->content) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected operator!");
        ret = 0;
      } else {
        BNFNode *node = newBNFNode(basenode, "", NODE_NOT);
        push(node->content, pop(subnode->content));
        push(subnode->content, &node);
        concat = 1;
      }
      oplast = 1;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->text[0] == '<') {
      BNFNode *node = parsebnfname(ss, basenode, trace, BNF_NOT_DEFINED);
      if (node) {
        push(content, &node);
        if (!expect(ss, trace, ">")) ret = 0;
      } else ret = 0;
      oplast = 0;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (!strcmp(s->text, stop) || (stop[0] == '\n' && s->type == SYMBOL_NEWLINE && !oplast)) {
      break;
    } //////////////////////////////////////////////////////////////////////////////////////////
    else if (s->type != SYMBOL_NEWLINE) {
      char error[256];
      sprintf(error, "Unexpected symbol '"FONT_BOLD"%s"FONT_RESET"'!", s->text);
      printsymbolmessage(ERRLVL_ERROR, trace, s, error);
      ret = 0;
    }
  } while(ret);

  if (content && !content->size) {
    printsymbolmessage(ERRLVL_WARNING, trace, s, "Empty group ignored");
    deleteBNFNode(pop(parent->content));
  } else if (content->size == 1) {
    bnfsimplify(parent, subnode, content);
  }
  return ret;
}

int parsebnfincludes(Parser *parser, SymbolStream *ss, BNFNode *basenode, Array *trace, Array *includes) {
  Symbol *s = NULL;
  if (trace->size > INCLUDE_MAX_DEPTH) {
    printfilemessage(ERRLVL_ERROR, trace, "Maximum include depth reached, check for recursive includes");
    return 0;
  }
  while ((s = ssgets(ss))) {
    if (s->type == SYMBOL_COMMENT) continue;
    if (strcmp(s->text, ";;") || s->type == SYMBOL_EOF) break;
    if (!expect(ss, trace, "include")) return 0;
    if (!expect(ss, trace, "("))       return 0;
    Symbol *n = newSymbol(&ss->symbol);
    String *a = newString("");
    while ((s = ssgets(ss))) {
      if (s->type == SYMBOL_NEWLINE) {
        printsymbolmessage(ERRLVL_ERROR, trace, n, "Expected closing parenthesis, reached newline instead");
        break;
      }
      if (s->type == SYMBOL_EOF) {
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
      printsymbolmessage(ERRLVL_WARNING, trace, n, "Expected file name inbetween parentheses");
    }
    deleteString(&a);
    deleteSymbol(&n);
    if (!expect(ss, trace, "\n")) return 0;
  }
  ssungets(s, ss);
  return 1;
}

void parsebnfbody(SymbolStream *ss, BNFNode *basenode, Array *trace)
{
  Symbol *s;
  while ((s = ssgets(ss))) {
    if (s->type == SYMBOL_EOF)     break;
    if (s->type == SYMBOL_NEWLINE) continue;
    if (s->type == SYMBOL_COMMENT) continue;
    if (strcmp(s->text, "<")) {
      char error[256];
      sprintf(error, "Expected '"FONT_BOLD"<"FONT_RESET"', got '"FONT_BOLD"%s"FONT_RESET"' instead!", s->text);
      printsymbolmessage(ERRLVL_ERROR, trace, s, error);
      break;
    } else {
      BNFNode *node = parsebnfname(ss, basenode, trace, BNF_DEFINED);
      if (!node)                     break;
      if (!expect(ss, trace, ">"))   break;
      push(basenode->content, &node);
      if (node->def == BNF_DECLARED) continue; // forward declared
      else if (node->def == BNF_DEFINED_AND_DECLARED) {
        node->def = BNF_DEFINED;
        continue;
      }
      if (!expect(ss, trace, "::=")) break;
      if (!parsebnfstatement(ss, basenode, node, trace, "\n")) break;
    }
  }
}

void verifybnftree(BNFNode *basenode, BNFNode *node, Array *trace)
{
  if (node != NULL) {
    if (node == basenode) {
      basenode->rec++;
    } else if (node->rec < basenode->rec) {
      node->rec = basenode->rec;
    } else return;
    if (node->type == NODE_ONE_OF && node->name[0] && node->def == BNF_NOT_DEFINED) {
      printnodemessage(ERRLVL_WARNING, trace, node->name, "Undefined node!");
    } else if (node->type != NODE_LEAF && node->type != NODE_RAW) {
      Array *content = node->content;
      for (int i = 0; i < content->size; i++) {
        verifybnftree(basenode, *(BNFNode**)at(content, i), trace);
      }
    }
  }
}

BNFNode *parsebnfnode(char *filename, Parser *parser, BNFNode *basenode, Array *trace, Array *includes)
{
  SymbolStream *ss = ssopen(filename, parser);
  push(trace, &filename);

  if (!ss) {
    printfilemessage(ERRLVL_ERROR, trace, "Could not open file!");
  } else {
    if (parsebnfincludes(parser, ss, basenode, trace, includes)) {
      parsebnfbody(ss, basenode, trace);
      verifybnftree(basenode, basenode, trace);
    }
    ssclose(ss);
  }

  pop(trace);
  return basenode;
}

void linkbnf(BNFNode *basenode, Array* trace) {
  Array *a = basenode->content;
  BNFNode *n;
  for (int i = 0; i < a->size;) {
    n = *(BNFNode**)pop(a);
    if (n != NULL && getnode(basenode, basenode, n->name) == NULL) {
      insert(a, 0, &n);
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
  if (node->type != NODE_LEAF && node->type != NODE_RAW) {
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