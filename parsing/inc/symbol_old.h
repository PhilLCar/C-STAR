#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

#include <array.h>
#include <diagnostic.h>
#include <tokenizer.h>
#include <tracked_stream.h>

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
  int          newline;
  TrackedFile *tfptr;
  Parser      *parser;
  Symbol       symbol;
  Array       *stack;
} SymbolStream;

typedef struct stringsymbolstream {
  String        *str;
  int            newline;
  TrackedString *tsptr;
  Parser        *parser;
  Symbol         symbol;
  SymbolType     previous;
  Array         *stack;
} StringSymbolStream;

typedef struct stream {
  void     *stream;
  int      *newline;
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
