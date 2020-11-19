#include <stdio.h>

#include <diagnostic.h>
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
  while (aboutc[++i].type != SYMBOL_COMMENT);
  while (aboutc[++i].type != SYMBOL_COMMENT);
  while (aboutc[++i].type != SYMBOL_COMMENT);

  CHECK_MEMORY;

  printsymbolmessage(ERRLVL_INFO, trace, &aboutc[i], "This is a test");
  printsuggest("Maybe try (%s) instead?", "this");

  CHECK_MEMORY;

  deleteArray(&trace);
  deleteParser(&parser);
  i = 0;
  int eof;
  do {
    eof = aboutc[i].type == SYMBOL_EOF;
    freesymbol(&aboutc[i++]);
  } while (!eof);
  free(aboutc);

  CHECK_MEMORY;
  STOP_WATCHING;

  return 0;
}
