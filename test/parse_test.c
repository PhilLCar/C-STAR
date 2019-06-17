#include <generic_parser.h>
#include <error.h>

void printsymbol(Symbol *symbol) {
  printf("Symbol: %s (line: %d, at %d)\n", symbol->text, symbol->line, symbol->position);
}

int main(void) {
  Parser *parser = newParser("parsing/prs/bnf.prs");
  Symbol *aboutc = parse("misc/c.txt", parser);

  printwarning("misc/c.txt", "wrong word", &aboutc[500]);
  printsuggest("Maybe try (%s) instead?", "this");
  return 0;
}
