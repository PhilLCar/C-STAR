#include <bnf_parser.h>

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
  }
}

void parseline(Node *n, SymbolStream *ss) {
  
}

int expect(SymbolStream *ss, char *str) {
  Symbol *s = getsymbol(ss);
  char error[256];
  
  if (strcmp(s->text, str)) {
    sprintf(error, "Expected '%s'!", str);
    printerror(ss->filename, error, s);
    return 0;
  }

  return 1;
}
