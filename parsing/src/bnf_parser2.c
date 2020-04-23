#include <bnf_parser2.h>

int expect(SymbolStream *ss, char *str) {
  char error[256];
  Symbol *s;

  if (!strcmp("\n", str)) {
    int junk = 0;
    char prev[256];
    sprintf(prev, "%s", ss->symbol.text);
    s = ssgets(ss);
    while (strcmp(s->text, str) && strcmp(s->text, "")) {
      s = ssgets(ss);
      junk = 1;
    }
    if (junk) {
      sprintf(error, "Junk after '"FONT_BOLD"%s"FONT_RESET"'", prev);
      printsymbolmessage(ERROR, trace, s, error);
    }
  }
  else {
    s = ssgets(ss);
    if (strcmp(s->text, str)) {
      sprintf(error, "Expected '"FONT_BOLD"%s"FONT_RESET"'!", str);
      printsymbolmessage(ERROR, trace, s, error);
      exit(1);
    }
  }

  return 1;
}

void parsebnfincludes(Parser *parser, SymbolStream *ss, Node *basenode, Array *trace) {
  Symbol *s = NULL;
  while (s = ssgets(ss)) {
    if (strcmp(s->text, ";;")) break;
    expect(ss, "include");
    expect(ss, "(");
    
  }
  ssungets(ss, s);
}

Node *parsebnfnode(char *filename, Parser *parser, Node *basenode, Array *trace)
{
  SymbolStream *ss = ssopen(filename, parser);

  push(trace, filename);
  parsebnfincludes(parser, ss, basenode, trace);
  parsebnfbody();
  pop(trace);

  ssclose(ss);
  return basenode;
}


Node *parsebnf(char *filename) 
{
  // TODO: Have parser saved to file for quicker recuperation
  Parser       *parser   = newParser("parsing/prs/bnf.prs");
  Node         *basenode;
  char          nodename[NODENAME_MAX_LENGTH];

  sprintf(nodename, "root:%s", filename);
  basenode = newNode(NULL, nodename, NODE_ROOT);

  
  deleteParser(&parser);
  return basenode;
}