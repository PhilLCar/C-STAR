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

#endif
