#include <bnf_parser.c>

Symbol *getsymbol(Symbols *s) {
  return s->symbol[s->i++];
}

Symbol *ungetsymbol(Symbols *s) {
  s->i--;
}

Node *getnode(Node *nodes, char *name) {
}

Node *parsebnf(Node *nodes, Symbols *symbols, int start, char *stop) {
  Node *node = malloc(sizeof(Node));
  for (int i = start; symbols[i] && symbols[i].text[0]; i++) {
    if (!strcmp(symbols[i].text, "{")) {
      parsebnf(symbols, i, "}");
    }
  }
}
