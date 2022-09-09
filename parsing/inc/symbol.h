#ifndef SYMBOL_PARSING
#define SYMBOL_PARSING

#include <array.h>
#include <diagnostic.h>
#include <oop.h>
#include <tokenizer.h>
#include <tracked_stream.h>

typedef enum symbol_type {
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
  SYMBOL_KEYWORD,
  SYMBOL_CONSTANT,
  SYMBOL_NEWLINE,
  SYMBOL_EOF
} SymbolType;

typedef struct symbol {
  String     *text;
  String     *open;
  String     *close;
  int         line;
  int         position;
  SymbolType  type;
} Symbol;

typedef struct symbol_stream {
  TrackedStream *ts;
  Tokenizer     *tokenizer;
  Array         *next;
} SymbolStream;

buildable(Symbol, symbol, const char *text);

// RETURNS a symbol stream from the generic stream <s>
SymbolStream *ssopen(Stream *s, Tokenizer *t);

// Closes the symbol stream <ss>
void          ssclose(SymbolStream *ss);

// RETURNS the next symbol in the tracked stream
Symbol       *ssgets(SymbolStream *ss);

// Ungets the symbol <s> on the symbol stream <ss>
void          ssungets(Symbol *s, SymbolStream *ss);

#endif
