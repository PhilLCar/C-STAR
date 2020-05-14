#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

#include <stdlib.h>

#include <diagnostic.h>
#include <generic_parser.h>
#include <tracked_file.h>
#include <tracked_string.h>
#include <array.h>

typedef enum symbolerror {
  SYMBOL_END_OF_FILE = 1,
  SYMBOL_ERROR_NONCLOSING,
  SYMBOL_ERROR_ILLEGAL_CHAR,
  SYMBOL_ERROR_OTHER
} SymbolError;

typedef enum symboltype {
  SYMBOL_NONE,
  SYMBOL_STRING,
  SYMBOL_COMMENT,
  SYMBOL_NUMBER,
  SYMBOL_VARIABLE,
  SYMBOL_OPERATOR,
  SYMBOL_RESERVED,
  SYMBOL_ERROR
} SymbolType;

typedef struct symbol {
  char       *text;
  char       *open;
  char       *close;
  int         line;
  int         position;
  SymbolType  type;
  int         eof;
} Symbol;

typedef struct symbolstream {
  char        *filename;
  TrackedFile *tfptr;
  Parser      *parser;
  Symbol       symbol;
  Array       *stack;
} SymbolStream;

typedef struct stringsymbolstream {
  String        *str;
  TrackedString *tsptr;
  Parser        *parser;
  Symbol         symbol;
  Array         *stack;
} StringSymbolStream;

Symbol       *sparse(char*, Parser*);
SymbolStream *ssopen(char*, Parser*);
void          ssclose(SymbolStream*);
Symbol       *ssgets(SymbolStream*);
void          ssungets(SymbolStream*, Symbol*);
Symbol       *newSymbol(Symbol*);
void          deleteSymbol(Symbol**);
void          freesymbol(Symbol*);

StringSymbolStream *sssopen(String*, Parser*);
void                sssclose(StringSymbolStream*);
Symbol             *sssgets(StringSymbolStream*);
void                sssungets(StringSymbolStream*, Symbol*);

#endif
