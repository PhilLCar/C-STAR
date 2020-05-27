#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

#include <stdlib.h>

#include <diagnostic.h>
#include <generic_parser.h>
#include <tracked_file.h>
#include <tracked_string.h>
#include <array.h>

typedef enum symboltype {
  SYMBOL_NONE,
  SYMBOL_STRING,
  SYMBOL_CHAR,
  SYMBOL_COMMENT,
  SYMBOL_INTEGER,
  SYMBOL_DECIMAL,
  SYMBOL_VARIABLE,
  SYMBOL_OPERATOR,
  SYMBOL_DELIMITER,
  SYMBOL_BREAK,
  SYMBOL_RESERVED,
  SYMBOL_CONSTANT,
  SYMBOL_NEWLINE,
  SYMBOL_EOF,

  SYMBOL_ERROR                = 0x100,
  SYMBOL_ERROR_NON_CLOSING    = 0x101
} SymbolType;

typedef struct symbol {
  char       *text;
  char       *open;
  char       *close;
  int         line;
  int         position;
  SymbolType  type;
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

typedef struct stream {
  void     *stream;
  Parser   *parser;
  Array    *stack;
  Symbol   *symbol;
  Symbol *(*gets)(void*);
  Symbol *(*ungets)(Symbol*, void*);
} Stream;

Symbol *sparse(char*, Parser*);
Symbol *newSymbol(Symbol*);
void    deleteSymbol(Symbol**);
void    freesymbol(Symbol*);

Stream *getStreamSS(SymbolStream*);
Stream *getStreamSSS(StringSymbolStream*);
void    closeStream(Stream*);

SymbolStream *ssopen(char*, Parser*);
void          ssclose(SymbolStream*);
Symbol       *ssgets(SymbolStream*);
void          ssungets(Symbol*, SymbolStream*);

StringSymbolStream *sssopen(String*, Parser*);
void                sssclose(StringSymbolStream*);
Symbol             *sssgets(StringSymbolStream*);
void                sssungets(Symbol*, StringSymbolStream*);

#endif
