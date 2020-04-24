#include <bnf_parser2.h>

BNFNode *parsebnfnode(char*, Parser*, BNFNode*, Array*);

BNFNode *newBNFNode(BNFNode *basenode, char *name, BNFType type) {
  if (!basenode) {
    if (type != NODE_ROOT) {
      printmessage(ERROR, "A non-root node was created with no parent!");
      exit(1);
    }
  } else {

  }
  return NULL;
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

BNFNode *parsebnfnodename(SymbolStream *ss, BNFNode *basenode, Array *trace) {
  Symbol *s = &ss->symbol;
  Symbol *t = newSymbol(s);
  s = ssgets(ss);
  if (s->eof) {
    printsymbolmessage(ERROR, trace, t, "Expected node name, reached end of file!");
  }
  if (!strcmp(s->text, "\n")) {
    printsymbolmessage(ERROR, trace, t, "Expected node name, reached new line!");
  }
  if (s->string || s->comment) {
    printsymbolmessage(ERROR, trace, s, "Bad format for node name!");
  }
  deleteSymbol(&t);
  return newBNFNode()
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

      expect(ss, trace, ">");
      expect(ss, trace, "::=");
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