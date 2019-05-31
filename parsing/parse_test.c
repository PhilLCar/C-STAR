#include "generic_parser.c"

int main(void) {
  Parser *parser = newParser("bnf.prs");
  printf("%p\n", parser->parentheses);
  return 0;
}
