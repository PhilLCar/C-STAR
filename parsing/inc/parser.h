#ifndef GENERIC_PARSING
#define GENERIC_PARSING

#include <stdio.h>

#include <diagnostic.h>

typedef struct parser {
  char   *whitespaces;
  char   *escapes;
  char  **strings;
  char  **chars;
  char  **linecom;
  char  **multicom;
  char  **operators;
  char  **delimiters;
  char  **breaksymbols;
  char  **reserved;
  char  **constants;
  int     lookahead;
} Parser;

Parser *newParser(char *filename);
void    deleteParser(Parser **parser);

#endif
