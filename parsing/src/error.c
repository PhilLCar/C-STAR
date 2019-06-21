#include <error.h>

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

void printerror(char *filename, char *error, Symbol *symbol) {
  char **context = getcontext(filename, symbol);
  if (!context) {
    fprintf(stderr, "Memory allocation error!");
    return;
  }
			      
  fprintf(stderr, TEXT_RED""FONT_BOLD"ERROR:"FONT_RESET
	  " In file "FONT_BOLD"%s"FONT_RESET": %s\n", filename, error);
  fprintf(stderr, "%s"FONT_BOLD""TEXT_MAGENTA"%s"FONT_RESET"%s\n", context[0], symbol->text, context[1]);

  for (int i = 0; context[0][i]; i++) fprintf(stderr, " ");
  for (int i = 0; symbol->text[i]; i++) fprintf(stderr, "^");
  fprintf(stderr, "\n");

  
  free(context[0]);
  free(context[1]);
  free(context);
}

void printwarning(char *filename, char *warning, Symbol *symbol) {
  char **context = getcontext(filename, symbol);
  if (!context) {
    fprintf(stderr, "Memory allocation warning!");
    return;
  }
			      
  fprintf(stderr, TEXT_YELLOW""FONT_BOLD"WARNING:"FONT_RESET
	  " In file "FONT_BOLD"%s"FONT_RESET" (line: %d, col: %d): %s\n", filename, symbol->line, symbol->position, warning);
  fprintf(stderr, "%s"FONT_BOLD""TEXT_MAGENTA"%s"FONT_RESET"%s\n", context[0], symbol->text, context[1]);

  for (int i = 0; context[0][i]; i++) fprintf(stderr, " ");
  for (int i = 0; symbol->text[i]; i++) fprintf(stderr, "~");
  fprintf(stderr, "\n");

  
  free(context[0]);
  free(context[1]);
  free(context);
}

void printsuggest(char *suggest, char *highlight) {
  char hl[256];
  sprintf(hl, TEXT_GREEN"%s"FONT_RESET, highlight);
  fprintf(stderr, suggest, hl);
  fprintf(stderr, "\n");
}
