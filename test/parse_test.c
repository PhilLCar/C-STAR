#include <symbol.h>
#include <error.h>

void printsymbol(Symbol *symbol) {
  printf("Symbol: %s (line: %d, at %d)\n", symbol->text, symbol->line, symbol->position);
}

int main(void) {
  Parser *parser = newParser("parsing/prs/bnf.prs");
  Symbol *aboutc = sparse("misc/c.txt", parser);

  int i = -1;
  while (!aboutc[++i].comment);
  while (!aboutc[++i].comment);
  while (!aboutc[++i].comment);
  printsymbolmessage(INFO, "misc/c.txt", NULL, "This is a test", &aboutc[i]);
  printsuggest("Maybe try (%s) instead?", "this");

  deleteParser(&parser);
  i = 0;
  do {
    freesymbol(&aboutc[i]);
  } while (aboutc[i].eof);
  free(aboutc);
  return 0;
}
