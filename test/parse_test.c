#include <symbol.h>
#include <error.h>

void printsymbol(Symbol *symbol) {
  printf("Symbol: %s (line: %d, at %d)\n", symbol->text, symbol->line, symbol->position);
}

int main(void) {
  Parser *parser = newParser("parsing/prs/bnf.prs");
  Symbol *aboutc = sparse("misc/c.txt", parser);
  Array  *trace  = newArray(sizeof(char*));
  char   *file = "misc/c.txt";
  
  push(trace, &file);
  int i = -1;
  while (!aboutc[++i].comment);
  while (!aboutc[++i].comment);
  while (!aboutc[++i].comment);
  printsymbolmessage(INFO, trace, &aboutc[i], "This is a test");
  printsuggest("Maybe try (%s) instead?", "this");

  deleteArray(&trace);
  deleteParser(&parser);
  i = 0;
  do {
    freesymbol(&aboutc[i]);
  } while (aboutc[i].eof);
  free(aboutc);
  return 0;
}
