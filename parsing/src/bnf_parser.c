#include <bnf_parser.h>

Node getnode(Node *basenode, char *nodename) {
  for (int i = 0; i < basenode->num; i++) {
    if (!strcmp(basenode->nodelist[i].name, nodename)) {
      return &basenode->nodelist[i];
    }
    else if (basenode->nodelist[i].type != NODE_LEAF) {
      Node *n = getnode(&basenode->nodelist[i], nodename);
      if (n != NULL) return n;
    }
  }
  return NULL;
}

void addnode(Node *basenode, Node *childnode) {
  
}

Node link(Node *basenode) {
}

Node *parsefile(char *filename) {
  // TODO: Have parser saved to file for quicker recuperation
  Parser       *parser   = newParser("parsing/prs/bnf.prs");
  SymbolStream *ss       = sopen(filename, parser);
  Node         *basenode = malloc(sizeof(Node));
  
  parseincludes(basenode, ss);
  parseline(basenode, ss);

  sclose(ss);
  deleteParser(&parser);

  return basenode;
}

void parseincludes(Node *n, SymbolStream *ss) {
  Symbol *s = getsymbol(ss);
  //char inc[256];
  if (!strcmp(s->text, ";;")) {
    expect(ss, "include");
    expect(ss, "(");
    s = getsymbol(ss);
    expect(")");
    expect("\n");
  }
}

void parseline(Node *n, SymbolStream *ss) {
  Symbol *s = getsymbol(ss);
  if (!strcmp(s->text, ";")) {
    while (strcmp(s->text, "\n")) s = getsymbol(ss);
  }
  expect(ss, "<");
  //getname
  expect(ss, ">");
  expect(ss, "::=");
  parsenode(n, ss, "\n");
}

void parsenode(Node *n, SymbolStream *ss, char *stop) {
  Symbol *s = getsymbol(ss);
  while (strcmp(s->text, stop)) {
    if (!strcmp(s->text, "(")) {
      parsenode(/**/, ss, ")");
    }
    else if (!strcmp(s->text, "{")) {
      parsenode(/**/, ss, "}");
    }
    else if (!strcmp(s->text, "[")) {
      parsenode(/**/, ss, "]");
    }
    else if (!strcmp(s->text, "'")) {
    }
    else if (!strcmp(s->text, "\"")) {
    }
    else if (!strcmp(s->text, "<")) {
    }
  }
}

int expect(SymbolStream *ss, char *str) {
  char error[256];
  Symbol *s;

  if (strcmp("\n", str)) {
    int junk = 0;
    while (strcmp(s, str)) {
      s = getsymbol(ss);
      junk = 1;
    }
    sprintf(
  }
  else if (strcmp(s->text, str)) {
    s = getsymbol(ss);
    sprintf(error, "Expected '%s'!", str);
    printerror(ss->filename, error, s);
    return 0;
  }

  return 1;
}
