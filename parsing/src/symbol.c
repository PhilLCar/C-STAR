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

// RETURNS index if peeked char is in a single char list, -1 otherwise
/********************************************************************************/
int isSingle(TrackedStream *ts, Tokenizer *tokenizer, SingleChar list)
{
  String *single = tokenizer->single_list[list];
  int     found  = -1;

  for (int i = 0; i < single->length; i++) {
    if (ts->buffer[0] == single->content[i]) {
      found = i;
      break;
    }
  }

  return found;
}

// RETURNS index if peeked sequence is in a multi char list, -1 otherwise
/********************************************************************************/
int isMulti(TrackedStream *ts, Tokenizer *tokenizer, MultiChar list)
{
  Array *multi = tokenizer->multi_list[list];
  int    best  =  0;
  int    found = -1;

  for (int i = 0; i < multi->size; i++) {
    String *str = *(String**)at(multi, i);
    int     cmp = strcmps(str->content, ts->buffer);
    /* NOTE: 
    It's to use <ts->buffer> in this case because its length is set by the
    maximum length of multi char list.
    */
    if (cmp > best) {
      best  = cmp;
      found = i;
      break;
    }
  }

  return found;
}

// RETURNS 1 if the next character on the stream is a newline, 0 otherwise
/********************************************************************************/
int isNewline(TrackedStream *ts) {
  return tspeek(ts, 0) == '\n';
}

// RETURNS 1 if the stream has ended, 0 otherwise
/********************************************************************************/
int isEOF(TrackedStream *ts) {
  return tspeek(ts, 0) == EOF;
}

// Records the current position of the stream <ts> into the <symbol>
/********************************************************************************/
void snap(TrackedStream *ts, Symbol *symbol)
{
  symbol->line     = ts->line;
  symbol->position = ts->position;
}

// Gets the next character on stream <ts>, and adds it to the <symbol>
/********************************************************************************/
void escape(TrackedStream *ts, Symbol *symbol)
{
  char c = tsgetc(ts);

  switch (c) {
    case 'n':
      append(symbol->text, '\n');
      break;
    case 't':
      append(symbol->text, '\t');
      break;
    case 'r':
      append(symbol->text, '\r');
      break;
    case '\\':
      append(symbol->text, '\\');
      break;
    case '\n':
      break;
    default:
    // TODO: (medium): Incomplete: Parse numeric escapes
      if (c > '0' && c <= '9') {
      } else if (c == '0') {
      }
      break;
  }
}

// Updates the symbol as a raw string
/********************************************************************************/
void symbolString(SymbolStream *ss, String *close, Symbol *symbol)
{
  TrackedStream *ts        = ss->ts;
  Tokenizer     *tokenizer = ss->tokenizer;

  while (!isEOF(ts) && strcmps(ts->buffer, close->content) != close->length) {
    char c = tsgetc(ts);

    if (c == tokenizer->escape) {
      escape(ts, symbol);
    } else append(symbol->text, c);
  }

  if (ts->buffer[0] == EOF) {
    // ERROR! (Non fatal) reached EOF
  } else {
    symbol->close = newString(close->content);
    for (int i = 0; i < close->length; i++) tsgetc(ts);
    symbol->type = SYMBOL_STRING;
  }
}

// Updates the symbol as a raw string
/********************************************************************************/
void symbolChar(SymbolStream *ss, String *close, Symbol *symbol)
{
  symbolString(ss, close, symbol);
  if (symbol->text->length != 1) {
    // ERROR! (Non fatal) a char should be only 1 character long
  } else {
    symbol->type = SYMBOL_CHAR;
  }
}

// Updates the symbol as a multiline comment
/********************************************************************************/
void symbolMultiline(SymbolStream *ss, String *close, Symbol *symbol)
{
  TrackedStream *ts        = ss->ts;
  Tokenizer     *tokenizer = ss->tokenizer;

  while (!isEOF(ts) && strcmps(ts->buffer, close->content) != close->length)
    append(symbol->text, tsgetc(ts));

  if (ts->buffer[0] == EOF) {
    // ERROR! (Non fatal) reached EOF
  } else {
    symbol->close = newString(close->content);
    for (int i = 0; i < close->length; i++) tsgetc(ts);
    symbol->type = SYMBOL_COMMENT;
  }
}

// TODO: (low): Think: Should escape allow multiline?
// Updates the symbol as a oneline comment
/********************************************************************************/
void symbolOneline(SymbolStream *ss, Symbol *symbol)
{
  TrackedStream *ts        = ss->ts;
  Tokenizer     *tokenizer = ss->tokenizer;

  while (!isEOF(ts) && !isNewline(ts)) append(symbol->text, tsgetc(ts));

  if (ts->buffer[0] == EOF) {
    // ERROR! (Non fatal) reached EOF
  } else {
    symbol->type = SYMBOL_COMMENT;
  }
}

////////////////////////////////////////////////////////////////////////////////
SymbolStream *ssopen(Stream *s, Tokenizer *t)
{
  SymbolStream *ss = malloc(sizeof(SymbolStream));
  if (ss) {
    ss->ts        = tsopen(s, t);
    ss->tokenizer = t;
    ss->next      = newArray(sizeof(Symbol*));

    if (!ss->ts || !t || !ss->next) {
      if (ss->ts) tsclose(ss->ts);
      deleteTokenizer(ss->tokenizer);
      deleteArray(&ss->next);
      free(ss);
    }
  }
  return ss;
}

////////////////////////////////////////////////////////////////////////////////
void ssclose(SymbolStream *ss)
{
  if (ss->ts) tsclose(ss->ts);
  deleteTokenizer(ss->tokenizer);
  deleteArray(&ss->next);
  free(ss);
}

////////////////////////////////////////////////////////////////////////////////
Symbol *ssgets(SymbolStream *ss)
{
  // Returns a symbol if one is on the outstack
  if (ss->next->size) return pop(ss->next);
  else {
    Tokenizer     *tokenizer = ss->tokenizer;
    TrackedStream *ts        = ss->ts;
    Symbol        *symbol    = newSymbol("");
    String        *text      = symbol->text;
    int            var       = 0;
    int            num       = 0;

    // Empties the whitespaces ahead of next symbol
    while (isSingle(ts, tokenizer, SINGLE_WHITESPACES) >= 0) tsgetc(ts);

    // BEGIN PARSING LOOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    do {
      MultiChar multi = MULTI_SIZE;
      int       match =  0;
      int       index = -1;
      char      peek0 = tspeek(ts, 0);
      char      peek1 = tspeek(ts, 1);

      // Check if the next symbol will be a number
      if (!text->length) {
        snap(ts, symbol); 
        num = (peek0 >= '0' && peek0 <= '9') || (peek1 >= '0' && peek1 <= '9' && peek0 == '.' );
      }

      // Should be smaller than the FIRST variable type so that they are not tested
      for (MultiChar mc = 0; mc < MULTI_RESERVED_KEYWORDS; mc++) {
        int i = isMulti(ts, tokenizer, mc);
        int m = 0;

        if (i > -1) {
          m = (*(String**)at(tokenizer->multi_list[mc], i))->length;
        }
        /* NOTE:
        There is a special case with decimals. '.' has to be taken out of consideration
        when the parsing is in a number context.
        */
        if (m > match && !(m == 1 && num && peek0 == '.')) {
          match = m;
          multi = mc;
          index = i;
        }
      }

      // An operator was found on the stream +++++++++++++++++++++++++++++++++++++++//
      if (match) {
        if (!text->length) {
          // TODO: (high): Incomplete: Finish cases
          switch (multi) {
          // OPEN/CLOSE
          //========================================================================//
          case MULTI_STRING_DELIMITERS:
          case MULTI_CHAR_DELIMITERS:
          case MULTI_MULTILINE_COMMENTS:
            if (index % 2) {
              // ERROR! (Not fatal) Not an opening delimiter // ignore
              for (int i = 0; i < match; i++) tsgetc(ts);
            } else {
              String *close = *(String**)at(tokenizer->multi_list[multi], index + 1);

              symbol->open = newString("");
              for (int i = 0; i < match; i++) append(symbol->open, tsgetc(ts));
              switch(match) {
                case MULTI_STRING_DELIMITERS:  symbolString   (ss, close, symbol); break;
                case MULTI_CHAR_DELIMITERS:    symbolChar     (ss, close, symbol); break;
                case MULTI_MULTILINE_COMMENTS: symbolMultiline(ss, close, symbol); break;
                default: break;
              }
            }
            break;
          // OPEN ONLY
          //========================================================================//
          case MULTI_ONELINE_COMMENTS:
            symbol->open = newString("");
            for (int i = 0; i < match; i++) append(symbol->open, tsgetc(ts));
            symbolOneline(ss, symbol);
            break;
          // NORMAL
          //========================================================================//
          case MULTI_SCOPE_DELIMITERS:
          case MULTI_OPERATORS:
          case MULTI_LINEBREAKS:
          default:
            for (int i = 0; i < match; i++) append(text, tsgetc(ts));
            switch(match) {
              case MULTI_SCOPE_DELIMITERS: symbol->type = SYMBOL_DELIMITER; break;
              case MULTI_OPERATORS:        symbol->type = SYMBOL_OPERATOR;  break;
              case MULTI_LINEBREAKS:       symbol->type = SYMBOL_BREAK;     break;
              default:                     symbol->type = SYMBOL_NONE;      break;
            }
            break;
          }
        }
        break;
      // An escape character was found on the stream +++++++++++++++++++++++++++++++//
      } else if (peek0 == tokenizer->escape) {
        if (!text->length) {
          tsgetc(ts); // consume escape
          if (tsgetc(ts) != '\n') {
            // ERROR! (Not fatal) Not an escapable character // ignore
          }
        } else break;
      // A newline/EOF was reached on the stream ++++++++++++++++++++++++++++++++++++++++//
      } else if (isNewline(ts) || isEOF(ts)) {
        if (!text->length) symbol->type = isNewline(ts) ? SYMBOL_NEWLINE : SYMBOL_EOF;
        break;
      // The symbol is of the variable/reserved type ++++++++++++++++++++++++++++++//
      } else {
        var = 1;
        append(text, tsgetc(ts));
      }
    } while (1);
    // END PARSING LOOP >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    // If it's a variable type, distinguish further
    if (var) {

    }

    return symbol;
  }
}

////////////////////////////////////////////////////////////////////////////////
void ssungets(Symbol *s, SymbolStream *ss)
{
  push(ss->next, &s);
}