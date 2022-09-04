#ifndef TOKENIZER_PARSING
#define TOKENIZER_PARSING

#include <stdio.h>

#include <array.h>
#include <diagnostic.h>
#include <oop.h>
#include <stream.h>

#define TOKENIZER_MIN_LOOKAHEAD 2

// The lists of single char symbols to be parsed
typedef enum single_char {
  SINGLE_WHITESPACES,
  SINGLE_ESCAPE_CHARS,
  SINGLE_SIZE
} SingleChar;

// The lists of multichar symbols to be parsed
typedef enum multi_char {
  // Punctuation types
  MULTI_STRING_DELIMITERS,
  MULTI_CHAR_DELIMITERS,
  MULTI_SCOPE_DELIMITERS,
  MULTI_ONELINE_COMMENTS,
  MULTI_MULTILINE_COMMENTS,
  MULTI_OPERATORS,
  MULTI_LINEBREAKS,
  // Variable types
  MULTI_RESERVED_KEYWORDS,
  MULTI_RESERVED_CONSTANTS,
  // Size of enum
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
