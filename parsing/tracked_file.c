#include "tracked_file.h"

TrackedFile *tfopen(char *filename, int size) {
  FILE *fptr = fopen(filename, "r");
  char *buffer = malloc((size + 1) * sizeof(char));
  TrackedFile *tf = malloc(sizeof(TrackedFile));
  
  if (fptr != NULL && buffer != NULL && tf != NULL) {
    memset(buffer, 0, (size + 1) * sizeof(char));
    tf->fptr     = fptr;
    tf->buffer   = buffer;
    tf->size     = size;
    tf->line     = -1;
    tf->position = -1;
  } else {
    if (fptr   != NULL) fclose(fptr);
    if (buffer != NULL) free(buffer);
    if (tf     != NULL) free(tf);
    tf = NULL;
  }
  return tf;
}

void tfclose(TrackedFile *tf) {
  fclose(tf->fptr);
  free(tf->buffer);
}

char tfgetc(TrackedFile *tf) {
  if (tf->line >= 0) {
    if (tf->buffer[0] == '\n') {
      tf->line++;
      tf->position = 0;
    } else tf->position++;
    for (int i = 0; i < tf->size - 1; i++) {
      tf->buffer[i] = tf->buffer[i + 1];
    }
    tf->buffer[tf->size - 1] = fgetc(tf->fptr);
  } else {
    for (int i = 0; i < tf->size; i++) {
      tf->buffer[i] = fgetc(tf->fptr);
    }
    tf->line = 0;
    tf->position = 0;
  }
  return tf->buffer[0];
}

void tfungetc(TrackedFile *tf, char c) {
  ungetc(tf->buffer[tf->size], tf->fptr);
  for (int i = tf->size; i > 0; i--) {
    tf->buffer[i] = tf->buffer[i - 1];
  }
  tf->buffer[0] = c;
}
