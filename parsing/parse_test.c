#include "generic_parser.c"

int main(void) {
  Parser *parser = newParser("bnf.prs");
  printf("%s\n", parser->parentheses[0][1]);
  return 0;
}
