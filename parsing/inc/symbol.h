#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

#include <diagnostic.h>
#include <parser.h>
#include <array.h>
#include <tracked_file.h>
#include <tracked_string.h>

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
  SYMBOL_NO_NEWLINE,
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

Symbol *sparse(char *filename, Parser *parser);
Symbol *newSymbol(Symbol *symbol);
void    deleteSymbol(Symbol **symbol);
void    freesymbol(Symbol *symbol);

Stream *getStreamSS(SymbolStream *ss);
Stream *getStreamSSS(StringSymbolStream *sss);
void    closeStream(Stream *s);

SymbolStream *ssopen(char *filename, Parser *parser);
void          ssclose(SymbolStream *ss);
Symbol       *ssgets(SymbolStream *ss);
void          ssungets(Symbol *symbol, SymbolStream *s);

StringSymbolStream *sssopen(String *string, Parser *parser);
void                sssclose(StringSymbolStream *sss);
Symbol             *sssgets(StringSymbolStream *sss);
void                sssungets(Symbol *symbol, StringSymbolStream *sss);

#endif
