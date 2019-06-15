#include "error.h"

char getline(char *filename, Symbol *symbol) {
  char *buffer = malloc(MAX_LINE_LENGTH * sizeof(char));
  FILE *fptr = fopen(filename, "r");

  if (buffer == NULL || fptr == NULL) {
  getline_error:
    if (buffer) free(buffer);
    if (fptr) fclose(fptr);
    return NULL;
  }
  
  memset(buffer, 0, MAX_LINE_LENGTH * sizeof(char));
  for (int i = 0; i < symbol->line; ) {
    char c = fgetc(fptr);
    if (c == '\n') i++;
    if (c == EOF) goto getline_error;
  }
  for (int i = 0; i < symbol->position; i++) {
    buffer
  }
}

void printerr(char *filename, char *error, Symbol *symbol) {
  fprintf(stderr, "In file "FONT_BOLD"%s"FONT_RESET": %s\n");
}
