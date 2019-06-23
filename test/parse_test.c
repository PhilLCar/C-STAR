#include <symbol.h>
#include <struct.h>
#include <error.h>

void printsymbol(Symbol *symbol) {
  printf("Symbol: %s (line: %d, at %d)\n", symbol->text, symbol->line, symbol->position);
}

int main(void) {
  Parser *parser = newParser("parsing/prs/bnf.prs");
  Symbol *aboutc = parse("misc/c.txt", parser);

  printsymbolmessage(INFO, "misc/c.txt", NULL, "This is a test", &aboutc[500]);
  printsuggest("Maybe try (%s) instead?", "this");

  savetofile("data/bnf_parser.struct", (void*)parser, sizeof(Parser));
  deleteParser(&parser);
  for (int i = 0; aboutc[i].text; i++) {
    free(aboutc[i].text);
  }
  free(aboutc);
  return 0;
}
