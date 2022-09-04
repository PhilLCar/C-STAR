#ifndef GENERIC_PARSING
#define GENERIC_PARSING

#include <stdio.h>

#include <array.h>
#include <diagnostic.h>
#include <oop.h>
#include <stream.h>

// The lists of single char symbols to be parsed
typedef enum single_char {
  SINGLE_WHITESPACES,
  SINGLE_ESCAPE_CHARS,
  SINGLE_SIZE
} SingleChar;

// The lists of multichar symbols to be parsed
typedef enum multi_char {
  MULTI_STRING_DELIMITERS,
  MULTI_CHAR_DELIMITERS,
  MULTI_SCOPE_DELIMITERS,
  MULTI_ONELINE_COMMENTS,
  MULTI_MULTILINE_COMMENTS,
  MULTI_OPERATORS,
  MULTI_LINEBREAKS,
  MULTI_RESERVED_KEYWORDS,
  MULTI_RESERVED_CONSTANTS,
  MULTI_SIZE
} MultiChar;

// Tokenizer parameters
typedef struct tokenizer {
  String *single_list[SINGLE_SIZE];
  Array  *multi_list[MULTI_SIZE];
  int     lookahead;
} Tokenizer;

buildable(Tokenizer, tokenizer, Stream *stream);

#endif
