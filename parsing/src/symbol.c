#include <symbol.h>

#include <stdlib.h>
#include <string.h>

// Compares the string until one ends
// RETURNS the match length, or 0 if no match
/********************************************************************************/
int strcmps(char *s1, char *s2)
{
  int i;
  for (i = 0;; i++) {
    if (s1[i] != s2[i]) {
      if (s1[i] && s2[i]) return 0;
      else                break;
    }
    if (!s1[i] || !s2[i]) break;
  }
  return i;
}

// RETURNS 1 if peeked char is in a single char list, 0 otherwise
/********************************************************************************/
int isSingle(TrackedStream *ts, Tokenizer *tokenizer, SingleChar list)
{
  String *single = tokenizer->single_list[list];
  int     found  = 0;

  for (int i = 0; i < single->length; i++) {
    if (ts->buffer[0] == single->content[i]) {
      found = 1;
      break;
    }
  }

  return found;
}

// RETURNS match length if peeked sequence is in a multi char list, 0 otherwise
/********************************************************************************/
int isMulti(TrackedStream *ts, Tokenizer *tokenizer, MultiChar list)
{
  Array *multi = tokenizer->multi_list[list];
  int    found = 0;

  for (int i = 0; i < multi->size; i++) {
    String *str = *(String**)at(multi, i);

    if ((found = strcmps(str->content, ts->buffer))) break;
  }

  return found;
}

// RETURNS 1 if the next character on the stream is a newline, 0 otherwise
/********************************************************************************/
int isNewline(TrackedStream *ts) {
  return ts->buffer[0] == '\n';
}

// RETURNS 1 if the stream has ended, 0 otherwise
/********************************************************************************/
int isEOF(TrackedStream *ts) {
  return ts->buffer[0] == EOF;
}

// RETURNS the next symbol in the tracked stream
/********************************************************************************/
int nextsymbol(TrackedStream *ts, Tokenizer *tokenizer, Symbol *symbol)
{
  String *text = newString("");
  int     var  = 0;
  int     num  = 0;

  // Empties the whitespaces ahead of next symbol
  while (isSingle(ts, tokenizer, SINGLE_WHITESPACES)) tsgetc(ts);

  do {
    int       match = 0;
    MultiChar multi = MULTI_SIZE;

    // Check if the next symbol will be a number
    if (!text->length && !(ts->buffer[0] == '.' && ts->buffer[1] >= '0' && ts->buffer[1] <= '9')) num = 1;

    // Should be smaller than the FIRST variable type so that they are not tested
    for (MultiChar i = 0; i < MULTI_RESERVED_KEYWORDS; i++) {
      int m = isMulti(ts, tokenizer, i);

      /* Decimal:
        There is a special case with decimals. '.' has to be taken out of consideration
        when the parsing is in a number context.
      */
      if (m > match && !(m == 1 && num && ts->buffer[0] == '.')) {
        match = m;
        multi = i;
      }
    }

    if (match) {
      if (!text->length) {
        switch (multi) {
        case MULTI_STRING_DELIMITERS:
          break;
        case MULTI_CHAR_DELIMITERS:
          break;
        case MULTI_SCOPE_DELIMITERS:
          break;
        case MULTI_ONELINE_COMMENTS:
          break;
        case MULTI_MULTILINE_COMMENTS:
          break;
        case MULTI_OPERATORS:
          break;
        case MULTI_LINEBREAKS:
          break;
        default:
          break;
        }
      }
      break;
    } else if (isSingle(ts, tokenizer, SINGLE_ESCAPE_CHARS)) {
      if (!text->length) {
        char c;

        tsgetc(ts);
        c = tsgetc;
        if (c != '\n') {
          // ERROR!
        }
      } else break;
    } else if (isNewline(ts)) {
      if (!text->length) symbol->type = SYMBOL_NEWLINE;
      break;
    } else if (isEOF(ts)) {
      if (!text->length) symbol->type = SYMBOL_EOF;
      break;
    } else {
      char c = tsgetc(ts);

      var = 1;
      append(text, tsgetc(ts));
      if (!num && c >= '0' && c <= '9') num = 1;
    }
  } while (1);

  // If it's a variable type, distinguish further
  if (var) {

  }

  return 1;
}
