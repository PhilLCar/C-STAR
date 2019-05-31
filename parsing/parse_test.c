#include "generic_parser.c"

int main(void) {
  Parser *parser = newParser("bnf.prs");
  printf("%s\n", parser->delimiters[3][0]);
  return 0;
}
