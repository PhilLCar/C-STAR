#include <symbol.h>

void freesymbol(Symbol *s)
{
  if (s->text)  free(s->text);
  if (s->open)  free(s->open);
  if (s->close) free(s->close);
}

// Compares the string until one ends (will return true in that case)
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

// Extends a buffer by one char, reallocating if need be
int extend(char **buffer, int *size, int *cap, char c)
{
  (*buffer)[(*size)++] = c;
  if (*size >= *cap) {
    char *t = realloc(*buffer, (*cap *= 2) * sizeof(char));
    if (t != NULL) {
      *buffer = t;
      memset(*buffer + *size, 0, (*cap - *size) * sizeof(char));
    } else return 0;
  }
  return 1;
}

// Returns the next symbol in the tracked file tf
int nextsymbol(TrackedFile *tf, Parser *parser, Symbol *symbol)
{
  char  c;
  int   buf_size = 0, buf_cap = 2;
  char *buf = malloc(buf_cap * sizeof(char));
  int   new = 0;
  int   pos = 0;

  symbol->open    = NULL;
  symbol->close   = NULL;
  symbol->string  = 0;
  symbol->comment = 0;
  symbol->eof     = 1;

  if (buf != NULL) {
    memset(buf, 0, buf_cap * sizeof(char));
    while ((c = tfgetc(tf))) {
      if (c == EOF) break;
      symbol->eof = 0;
      int cmp, ws = 0;
      //////////////////////////////////////// NEW-LINE ////////////////////////////////////////
      if (c == '\n') {
        if (buf_size) {
          tfungetc(tf, c);
        } else {
          pos = tf->position;
          if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
        }
        break;
      }
      //////////////////////////////////////// ESCAPE ////////////////////////////////////////
      for (int i = 0; parser->escapes[i]; i++) {
        if (c == parser->escapes[i]) {
          ws = 1;
          break;
        }
      }
      if (ws) {
        c = tfgetc(tf);
        if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
        continue;
      }
      //////////////////////////////////////// WHITESPACE ////////////////////////////////////////
      for (int i = 0; parser->whitespaces[i]; i++) {
        if (c == parser->whitespaces[i]) {
          ws = 1;
          break;
        }
      }
      if (ws) {
        if (buf_size) break;
        else          continue;
      }
      //////////////////////////////////////// DELIMITERS ////////////////////////////////////////
      for (int i = 0; parser->delimiters[i]; i++) {
        if (i % 2) continue;
        if ((cmp = strcmps(tf->buffer, parser->delimiters[i]))) {
          if (cmp > ws) ws = cmp;
        }
      }
      if (ws) {
        if (buf_size) {
          tfungetc(tf, c);
        } else {
          //_symbol_mode_string = 1;
          pos = tf->position;
          for (int i = 1;; i++) {
            if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
            if (i == ws) break;
            c = tfgetc(tf);
          }
        }
        break;
      }
      //////////////////////////////////////// BREAKSYMBOLS ////////////////////////////////////////
      for (int i = 0; parser->breaksymbols[i]; i++) {
        if ((cmp = strcmps(tf->buffer, parser->breaksymbols[i]))) {
          if (cmp > ws) ws = cmp;
        }
      }
      if (ws) {
        if (buf_size) {
          tfungetc(tf, c);
        } else {
          pos = tf->position;
          for (int i = 1;; i++) {
            if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
            if (i == ws) break;
            c = tfgetc(tf);
          }
        }
        break;
      }
      //////////////////////////////////////// SYMBOL ////////////////////////////////////////
      if (!buf_size) pos = tf->position;
      if (!extend(&buf, &buf_size, &buf_cap, c)) {
      next_fail:
        free(buf);
        return 0;
      }
    }
    symbol->text = buf;
    symbol->line = tf->line;
    symbol->position = pos;
    new = 1;
  }
  return new;
}

// Parses a file using the rules provided in parser
Symbol *sparse(char *filename, Parser *parser)
{
  TrackedFile *tf = tfopen(filename, parser->lookahead);
  int symbols_size = 0, symbols_cap = 1024;
  Symbol *symbols = NULL;
  if (tf != NULL) {
    symbols = malloc(symbols_cap * sizeof(Symbol));
    if (symbols != NULL) {
      memset(symbols, 0, symbols_cap * sizeof(Symbol));
      while (nextsymbol(tf, parser, &symbols[symbols_size])) {
        if (symbols[symbols_size].eof) {
          // END OF FILE
          break;
        }
        if (++symbols_size == symbols_cap) {
          Symbol *t = realloc(symbols, (symbols_cap *= 2) * sizeof(Symbol));
          if (t != NULL) {
            symbols = t;
            memset(symbols + symbols_size, 0, (symbols_cap - symbols_size) * sizeof(Symbol));
          }
          else break;
        }
      }
    }
    tfclose(tf);
  }
  return symbols;
}

SymbolStream *ssopen(char *filename, Parser *parser)
{
  SymbolStream *ss    = malloc(sizeof(SymbolStream));
  TrackedFile  *tf    = tfopen(filename, parser->lookahead);
  Array        *stack = newArray(sizeof(Symbol));

  if (ss != NULL && parser != NULL && stack != NULL) {
    ss->filename = filename;
    ss->tfptr    = tf;
    ss->parser   = parser;
    ss->stack    = stack;

    ss->symbol.text  = NULL;
    ss->symbol.open  = NULL;
    ss->symbol.close = NULL;
  }
  else
  {
    ssclose(ss);
    ss = NULL;
  }
  return ss;
}

void ssclose(SymbolStream *ss)
{
  Symbol *s;
  while ((s = (Symbol*)pop(ss->stack))) freesymbol(s);
  if (ss->tfptr) tfclose(ss->tfptr);
  deleteArray(&ss->stack);
  freesymbol(&ss->symbol);
  free(ss);
}

Symbol *gets(SymbolStream *ss)
{
  Symbol *s = &ss->symbol;
  if (ss->stack->size) {
    memcpy(s, pop(ss->stack), sizeof(Symbol));
  } else {
    freesymbol(s);
    if (!nextsymbol(ss->tfptr, ss->parser, s)) {
      s = NULL;
    }
  }
  return s;
}

void ungets(SymbolStream *ss, Symbol *s)
{
  push(ss->stack, s);
}

void deleteSymbol(Symbol **s)
{
  if (*s != NULL) {
    if ((*s)->text)  free((*s)->text);
    if ((*s)->open)  free((*s)->open);
    if ((*s)->close) free((*s)->close);
    free(*s);
    *s = NULL;
  }
}
