#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

#include <diagnostic.h>
#include <generic_parser.h>
#include <tracked_file.h>
#include <array.h>

typedef struct symbol {
  char *text;
  char *open;
  char *close;
  int   line;
  int   position;
  int   string;
  int   comment;
  int   eof;
} Symbol;

typedef struct symbolstream {
  char        *filename;
  TrackedFile *tfptr;
  Parser      *parser;
  Symbol       symbol;
  Array       *stack;
} SymbolStream;

Symbol       *sparse(char*, Parser*);
SymbolStream *ssopen(char*, Parser*);
void          ssclose(SymbolStream*);
Symbol       *ssgets(SymbolStream*);
void          ssungets(SymbolStream*, Symbol*);
Symbol       *newSymbol(Symbol*);
void          deleteSymbol(Symbol**);
void          freesymbol(Symbol*);

#endif
