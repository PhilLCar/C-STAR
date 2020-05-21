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
  char  **strings;
  char  **chars;
  char  **linecom;
  char  **multicom;
  char  **operators;
  char  **delimiters;
  char  **breaksymbols;
  char  **reserved;
  int     lookahead;
} Parser;

char   *characters(FILE*);
char  **word(FILE*);
void    emptyline(FILE*);

Parser *newParser(char*);
void    deleteParser(Parser**);

#endif
