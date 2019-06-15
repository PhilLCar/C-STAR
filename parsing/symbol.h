#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

typedef struct symbol {
  char *text;
  int   line;
  int   position;
} Symbol;

#endif
