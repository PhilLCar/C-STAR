#include <error.h>

#include <string.h>
#include <stdlib.h>

#include <macro.h>
#include <terminal.h>

// find better algorithm for no memory r-rotation
void rotate(char *buffer, int s, int r) {
  char c;
  for (int j = 0; j < r; j++) {
    c = buffer[0];
    for (int i = 0; i < s - 1;i++) {
      buffer[i] = buffer[i + 1];
    }
    buffer[s - 1] = c;
  }
}

char **getcontext(char *filename, Symbol *symbol) {
  char **context = malloc(2 * sizeof(char*));
  char  *before  = malloc((CONTEXT_LENGTH + 1) * sizeof(char));
  char  *after   = malloc((CONTEXT_LENGTH + 1) * sizeof(char));
  FILE  *fptr    = fopen(filename, "r");

  if (context == NULL || before == NULL || after == NULL || fptr == NULL) {
  getline_error:
    if (context) free(context);
    if (before)  free(before);
    if (after)   free(after);
    if (fptr)    fclose(fptr);
    return NULL;
  }
  
  memset(before, 0, (CONTEXT_LENGTH + 1) * sizeof(char));
  memset(after,  0, (CONTEXT_LENGTH + 1) * sizeof(char));
  for (int i = 0; i < symbol->line; ) {
    char c = fgetc(fptr);
    if (c == '\n') i++;
    if (c == EOF) goto getline_error;
  }
  int l;
  for (l = 0; l < symbol->position; l++) {
    before[l % CONTEXT_LENGTH] = fgetc(fptr);
  }
  if (l >= CONTEXT_LENGTH) {
    rotate(before, CONTEXT_LENGTH, l);
    for (int i = 0; i < 3; i++) before[i] = '.';
  }
  for (int i = 0; symbol->text[i]; i++) {
    fgetc(fptr);
  }
  for (l = 0; l < CONTEXT_LENGTH;l++) {
    char c = fgetc(fptr);
    if (c == '\n' || c == EOF) break;
    after[l] = c;
  }
  if (l >= CONTEXT_LENGTH) {
    for (int i = 3; i > 0; i--) after[CONTEXT_LENGTH - i] = '.';
  }
  
  context[0] = before;
  context[1] = after;
  fclose(fptr);
  return context;
}

void printtrace(Array *trace) {
  if (trace->size > 1) {
    fprintf(stderr, "In file included from "FONT_BOLD"%s"FONT_RESET"\n", *(char**)at(trace, trace->size - 2));
    for (int i = trace->size - 3; i >= 0; i--) {
      fprintf(stderr, "                 from "FONT_BOLD"%s"FONT_RESET"\n", *(char**)at(trace, i));
    }
  }
}

void printmtrace(Array *hist) {
  if (hist->size > 1) {
    Expanded *exp = at(hist, hist->size - 2);
    fprintf(stderr, "In macro expansion of "FONT_BOLD"%s"FONT_RESET"\n", exp->m->name->content);
    for (int i = hist->size - 3; i >= 0; i--) {
      exp = at(hist, i);
      fprintf(stderr, "        expanded from "FONT_BOLD"%s"FONT_RESET"\n", exp->m->name->content);
    }
  }
}

void printmessagetype(MessageType type) {
  switch (type) {
  case ERRLVL_INFO:
    fprintf(stderr, TEXT_GREEN""FONT_BOLD"INFO: "FONT_RESET);
    break;
  case ERRLVL_DEBUG:
    fprintf(stderr, TEXT_BLUE""FONT_BOLD"DEBUG: "FONT_RESET);
    break;
  case ERRLVL_WARNING:
    fprintf(stderr, TEXT_YELLOW""FONT_BOLD"WARNING: "FONT_RESET);
    break;
  case ERRLVL_ERROR:
    fprintf(stderr, TEXT_RED""FONT_BOLD"ERROR: "FONT_RESET);
    break;
  }
}

void printfilename(char *filename) {
  if (strcmp(filename, "")) {
    fprintf(stderr, "In file "FONT_BOLD"%s"FONT_RESET": ", filename);
  }
}

void printnodename(char *nodename) {
  if (strcmp(nodename, "")) {
    fprintf(stderr, "Node "FONT_BOLD"<%s>"FONT_RESET": ", nodename);
  }
}

void printcoords(Symbol *symbol) {
  fprintf(stderr, "%d: %d: ", symbol->line, symbol->position);
}

void printcontext(MessageType type, Symbol *symbol, char *filename) {
  char **context = getcontext(filename, symbol);
  if (!context) {
    fprintf(stderr, "Context not available! (Probably reached end of file)\n");
    return;
  }
  
  fprintf(stderr, "%s"FONT_BOLD""TEXT_MAGENTA"%s"FONT_RESET"%s\n", context[0], symbol->text, context[1]);
  
  for (int i = 0; context[0][i]; i++) fprintf(stderr, " ");
  switch (type) {
  case ERRLVL_INFO:
    for (int i = 0; symbol->text[i]; i++) fprintf(stderr, ".");
    break;
  case ERRLVL_DEBUG:
    for (int i = 0; symbol->text[i]; i++) fprintf(stderr, "-");
    break;
  case ERRLVL_WARNING:
    for (int i = 0; symbol->text[i]; i++) fprintf(stderr, "~");
    break;
  case ERRLVL_ERROR:
    for (int i = 0; symbol->text[i]; i++) fprintf(stderr, "^");
    break;
  }
  fprintf(stderr, "\n");

  
  free(context[0]);
  free(context[1]);
  free(context);
}

void printirmessage(MessageType type, char *metadata, Symbol *symbol, char *message) {
  Symbol       *tmp      = newSymbol(symbol);
  Parser       *parser   = newParser("parsing/prs/meta.prs");
  Array        *trace    = newArray(sizeof(char*));
  SymbolStream *ss       = ssopen(metadata, parser);
  int           found    = 0;

  while (!found) {
    ssgets(ss);
    switch(ss->symbol.type) {
      case SYMBOL_RESERVED:
        if (!strcmp(ss->symbol.text, "#file")) {
          String *s;
          ssgets(ss);
          s = newString(ss->symbol.text);
          push(trace, &s->content);
          s->content = NULL;
          deleteString(&s);
        } else { // #pop
          free(*(char**)pop(trace));
        }
        ssgets(ss); // newline
        break;
      case SYMBOL_INTEGER:
        {
          int fline = atoi(ss->symbol.text);
          int fposition, nline, nposition;
          ssgets(ss); // :
          ssgets(ss);
          fposition = atoi(ss->symbol.text);
          ssgets(ss); // <-
          ssgets(ss);
          nline     = atoi(ss->symbol.text);
          ssgets(ss); // :
          ssgets(ss);
          nposition = atoi(ss->symbol.text);
          ssgets(ss); // newline
          if (fposition >= tmp->position && fline >= tmp->line) {
            tmp->line     = nline;
            tmp->position = nposition;
            printsymbolmessage(type, trace, tmp, message);
            found = 1;
          }
        }
        break;
      default:
        fprintf(stderr, "There was an error parsing metadata, the help messages might not be accurate...\n");
        break;
    }
  }

  while (trace->size) {
    free(*(char**)pop(trace));
  }

  ssclose(ss);
  deleteArray(&trace);
  deleteParser(&parser);
  deleteSymbol(&tmp);
}

void printmacromessage(MessageType type, Array *trace, Array *hist, char *message) {
  Macro  *macro = ((Expanded*)at(hist, hist->size - 1))->m;
  Symbol *s     = malloc(sizeof(Symbol));
  s->open       = NULL;
  s->close      = NULL;
  s->text       = macro->name->content;
  s->position   = macro->position;
  s->line       = macro->line;

  printtrace(trace);
  printmessagetype(type);
  printfilename(*(char**)at(trace, trace->size - 1));
  printcoords(s);
  printmtrace(hist);
  fprintf(stderr, "%s\n", message);
  printcontext(type, s, macro->filename->content);
  free(s);
}

void printnodemessage(MessageType type, Array *trace, char *nodename, char* message) {
  printtrace(trace);
  printmessagetype(type);
  printfilename(*(char**)at(trace, trace->size - 1));
  printnodename(nodename);
  fprintf(stderr, "%s\n", message);
}

void printsymbolmessage(MessageType type, Array *trace, Symbol *symbol, char *message) {
  printtrace(trace);
  printmessagetype(type);
  printfilename(*(char**)at(trace, trace->size - 1));
  printcoords(symbol);
  fprintf(stderr, "%s\n", message);
  printcontext(type, symbol, *(char**)at(trace, trace->size - 1));
}
      
void printfilemessage(MessageType type, Array *trace, char *message) {
  printtrace(trace);
  printmessagetype(type);
  printfilename(*(char**)at(trace, trace->size - 1));
  fprintf(stderr, "%s\n", message);
}

void printmessage(MessageType type, char *message) {
  printmessagetype(type);
  fprintf(stderr, "%s\n", message);
}

void printsuggest(char *suggest, char *highlight) {
  char hl[256];
  sprintf(hl, TEXT_GREEN"%s"FONT_RESET, highlight);
  fprintf(stderr, suggest, hl);
  fprintf(stderr, "\n");
}
