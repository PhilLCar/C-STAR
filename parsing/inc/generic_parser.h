#ifndef GENERIC_PARSING
#define GENERIC_PARSING

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <diagnostic.h>
#include <tracked_file.h>

typedef struct parser {
  char   *whitespaces;
  char   *escapes;
  char  **delimiters;
  char  **linecom;
  char  **multicom;
  char  **breaksymbols;
  int     lookahead;
} Parser;

char   *characters(FILE*);
char  **word(FILE*);
void    emptyline(FILE*);

Parser *newParser(char*);
void    deleteParser(Parser**);

#endif
