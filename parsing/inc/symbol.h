#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

#include "generic_parser.h"
#include "tracked_file.h"

typedef struct symbol {
  char *text;
  int   line;
  int   position;
} Symbol;

typedef struct symbolstream {
  char        *filename;
  TrackedFile *tfptr;
  Parser      *parser;
  Symbol      *symbol;
} SymbolStream;

int           nextsymbol(TrackedFile*, Parser*, Symbol*);
Symbol       *parse(char*, Parser*);

SymbolStream *sopen(char*, Parser*);
void          sclose(SymbolStream*);
Symbol       *getsymbol(SymbolStream*);

#endif
