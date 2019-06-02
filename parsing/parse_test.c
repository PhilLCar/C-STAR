#include "generic_parser.c"

int main(void) {
  Parser *parser = newParser("bnf.prs");
  printf("%s\n", parser->parentheses[2][1]);
  printf("%d\n", parser->max_depth);
  return 0;
}
