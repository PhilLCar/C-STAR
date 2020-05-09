#include <symbol.h>

enum {
  NONE = 0,
  DELIM,
  LINECOM,
  MULTICOM,
  BREAK
};

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
  int   type, close = 0;
  int   new = 0, pos = 0;

  symbol->text    = NULL;
  symbol->open    = NULL;
  symbol->close   = NULL;
  symbol->line    = -1;
  symbol->type    = SYMBOL_NONE;
  symbol->eof     = 1;

  if (buf != NULL) {
    memset(buf, 0, buf_cap * sizeof(char));
    while ((c = tfgetc(tf)) != EOF) {
      type = NONE;
      symbol->eof = 0;
      int cmp, tmp, ws = 0;
      //////////////////////////////////////// NEW-LINE ////////////////////////////////////////
      if (c == '\n' && symbol->type != SYMBOL_COMMENT) {
        if (buf_size) {
          tfungetc(tf, c);
        } else {
          pos = tf->position;
          if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
        }
        break;
      }
      //////////////////////////////////////// STRING STOP ////////////////////////////////////////
      if (symbol->type == SYMBOL_STRING) {
        //////////////////////////////////////// ESCAPE ////////////////////////////////////////
        for (int i = 0; parser->escapes[i]; i++) {
          if (c == parser->escapes[i]) {
            ws = 1;
            break;
          }
        }
        if (ws) {
          c = tfgetc(tf);
          switch (c) {
            case 'n':
              c = '\n';
              break;
            case 't':
              c = '\t';
              break;
            case 'r':
              c = '\r';
              break;
            case '0':
              c = '\0';
              break;
          }
          if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
          continue;
        }
        if ((cmp = strcmps(tf->buffer, symbol->close))) {
          close = 1;
          for (int i = 1; i < cmp; i++) tfgetc(tf);
          break;
        } else if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
        continue;
      }
      //////////////////////////////////////// COM STOP ////////////////////////////////////////
      if (symbol->type == SYMBOL_COMMENT) {
        if (!symbol->close && c == '\n') {
          tfungetc(tf, c);
          break;
        } else if (symbol->close && (cmp = strcmps(tf->buffer, symbol->close))) {
          close = 1;
          for (int i = 1; i < cmp; i++) tfgetc(tf);
          break;
        } else if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
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
      for (int i = 0; parser->delimiters[i]; i += 2) {
        if ((cmp = strcmps(tf->buffer, parser->delimiters[i]))) {
          if (cmp > ws) {
            ws  = cmp;
            tmp = i;
            type = DELIM;
          }
        }
      }
      //////////////////////////////////////// LINECOM ////////////////////////////////////////
      for (int i = 0; parser->linecom[i]; i++) {
        if ((cmp = strcmps(tf->buffer, parser->linecom[i]))) {
          if (cmp > ws) {
            ws  = cmp;
            tmp = i;
            type = LINECOM;
          }
        }
      }
      //////////////////////////////////////// MULTICOM ////////////////////////////////////////
      for (int i = 0; parser->multicom[i]; i += 2) {
        if ((cmp = strcmps(tf->buffer, parser->multicom[i]))) {
          if (cmp > ws) {
            ws  = cmp;
            tmp = i;
            type = MULTICOM;
          }
        }
      }
      //////////////////////////////////////// BREAKSYMBOLS ////////////////////////////////////////
      for (int i = 0; parser->breaksymbols[i]; i++) {
        if ((cmp = strcmps(tf->buffer, parser->breaksymbols[i]))) {
          if (cmp > ws) {
            ws = cmp;
            type = BREAK;
          }
        }
      }
      //////////////////////////////////////// NUMBERS ////////////////////////////////////////
      if (!buf_size) {
        int dec = 0;
        if (c == '.') {
          char t = tfgetc(tf);
          if (t >= '0' && t <= '9') dec = 1;
          tfungetc(tf, t);
        }
        if (dec || (c >= '0' && c <= '9')) {
          symbol->type = SYMBOL_NUMBER;
          if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
          continue;
        }
      } else if (symbol->type == SYMBOL_NUMBER && type == NONE) {
        if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
        continue;
      }
      //////////////////////////////////////////////////////////////////////////////////////////
      // delim
      if (ws && type == DELIM) {
        if (buf_size) {
          tfungetc(tf, c);
          break;
        } else {
          symbol->type   = SYMBOL_STRING;
          symbol->line   = tf->line;
          symbol->open   = malloc((parser->lookahead + 1) * sizeof(char));
          symbol->close  = malloc((parser->lookahead + 1) * sizeof(char));
          if (symbol->open != NULL && symbol->close != NULL) {
            memcpy(symbol->open,  parser->delimiters[tmp],     ws                                  + 1);
            memcpy(symbol->close, parser->delimiters[tmp + 1], strlen(parser->delimiters[tmp + 1]) + 1);
          } else {
            goto next_fail;
          }
          pos = tf->position + ws;
          for (int i = 1; i < ws; i++) {
            c = tfgetc(tf);
          }
          continue;
        }
      }
      // linecom
      if (ws && type == LINECOM) {
        if (buf_size) {
          tfungetc(tf, c);
          break;
        } else {
          symbol->type    = SYMBOL_COMMENT;
          symbol->open    = malloc((parser->lookahead + 1) * sizeof(char));
          if (symbol->open != NULL) {
            memcpy(symbol->open,  parser->linecom[tmp], ws + 1);
          } else {
            goto next_fail;
          }
          pos = tf->position + ws;
          for (int i = 1; i < ws; i++) {
            c = tfgetc(tf);
          }
          continue;
        }
      }
      // multicom
      if (ws && type == MULTICOM) {
        if (buf_size) {
          tfungetc(tf, c);
          break;
        } else {
          symbol->type    = SYMBOL_COMMENT;
          symbol->line    = tf->line;
          symbol->open    = malloc((parser->lookahead + 1) * sizeof(char));
          symbol->close   = malloc((parser->lookahead + 1) * sizeof(char));
          if (symbol->open != NULL && symbol->close != NULL) {
            memcpy(symbol->open,  parser->multicom[tmp],     ws                                + 1);
            memcpy(symbol->close, parser->multicom[tmp + 1], strlen(parser->multicom[tmp + 1]) + 1);
          } else {
            goto next_fail;
          }
          pos = tf->position + ws;
          for (int i = 1; i < ws; i++) {
            c = tfgetc(tf);
          }
          continue;
        }
      }
      // break
      if (ws && type == BREAK) {
        if (buf_size) {
          tfungetc(tf, c);
        } else {
          symbol->type = SYMBOL_OPERATOR;
          pos          = tf->position;
          for (int i = 1;; i++) {
            if (!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
            if (i == ws) break;
            c = tfgetc(tf);
          }
        }
        break;
      }
      //////////////////////////////////////// SYMBOL ////////////////////////////////////////
      if (!buf_size) {
        symbol->type = SYMBOL_VARIABLE;
        pos          = tf->position;
      }
      if (!extend(&buf, &buf_size, &buf_cap, c)) {
      next_fail:
        freesymbol(symbol);
        free(buf);
        return 0;
      }
    }
    if (!close && (symbol->type == SYMBOL_STRING ||
                  (symbol->type == SYMBOL_COMMENT && symbol->close))) {
      symbol->close[0] = 0;
    }
    symbol->text = buf;
    for (int i = 0; parser->reserved[i]; i++) {
      if (!strcmp(buf, parser->reserved[i])) {
        symbol->type = SYMBOL_RESERVED;
        break;
      }
    }
    if (symbol->line < 0) symbol->line = tf->line;
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

  if (parser && ss && tf && stack) {
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
    if (ss)    free(ss);
    if (tf)    tfclose(tf);
    if (stack) deleteArray(&stack);
    ss = NULL;
  }
  return ss;
}

void ssclose(SymbolStream *ss)
{
  while (popobj(ss->stack, (F)freesymbol));
  if (ss->tfptr) tfclose(ss->tfptr);
  deleteArray(&ss->stack);
  freesymbol(&ss->symbol);
  free(ss);
}

Symbol *ssgets(SymbolStream *ss)
{
  Symbol *s = &ss->symbol;
  freesymbol(s);
  if (ss->stack->size) {
    *s = *(Symbol*)pop(ss->stack);
  } else {
    if (!nextsymbol(ss->tfptr, ss->parser, s)) {
      return NULL;
    }
  }
  return s;
}

void ssungets(SymbolStream *ss, Symbol *s)
{
  Symbol *t = newSymbol(s);
  push(ss->stack, t);
  free(t);
}

Symbol *newSymbol(Symbol *s)
{
  Symbol *n = malloc(sizeof(Symbol));
  if (n) {
    memcpy(n, s, sizeof(Symbol));
    n->text                = malloc((strlen(s->text)  + 1) * sizeof(char));
    if (s->open)  n->open  = malloc((strlen(s->open)  + 1) * sizeof(char));
    else          n->open  = NULL;
    if (n->close) n->close = malloc((strlen(s->close) + 1) * sizeof(char));
    else          n->close = NULL;

    if (n->text) {
                    memcpy(n->text,  s->text,  strlen(s->text)  + 1);
      if (n->open)  memcpy(n->open,  s->open,  strlen(s->open)  + 1);
      if (n->close) memcpy(n->close, s->close, strlen(s->close) + 1);
    } else {
      deleteSymbol(&n);
    }
  }
  return n;
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

void freesymbol(Symbol *s)
{
  if (s->text)  free(s->text);
  if (s->open)  free(s->open);
  if (s->close) free(s->close);
}