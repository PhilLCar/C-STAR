#include "tracked_file.h"

void push(TrackedFile tf, int pos) {
  tf->linestack[tf->stack_size
}

int pop(TrackedFile tf) {
  
}

TrackedFile *tfopen(char *filename, int size) {
  FILE        *fptr   = fopen(filename, "r");
  char        *buffer = malloc((size + 1) * sizeof(char));
  int          cap    = 16;
  int         *stack  = malloc(cap * sizeof(int));
  TrackedFile *tf     = malloc(sizeof(TrackedFile));
  
  if (fptr != NULL && buffer != NULL && stack != NULL && tf != NULL && size) {
    memset(buffer, 0, (size + 1) * sizeof(char));
    
    tf->fptr     = fptr;
    tf->buffer   = buffer;
    tf->size     = size;
    tf->line     = 0;
    tf->position = -1;

    tf->linestack  = stack;
    tf->stack_cap  = cap;
    tf->stack_size = 0;
  } else {
    if (fptr   != NULL) fclose(fptr);
    if (buffer != NULL) free(buffer);
    if (stack  != NULL) free(stack);
    if (tf     != NULL) free(tf);
    tf = NULL;
  }
  return tf;
}

void tfclose(TrackedFile *tf) {
  fclose(tf->fptr);
  free(tf->buffer);
  free(tf->linestack);
}

char tfgetc(TrackedFile *tf) {
  if (tf->position >= 0) {
    if (tf->buffer[0] == '\n') {
      tf->line++;
      push(tf->position);
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
  if (c == '\n') {
    tf->line--;
    tf->position = pop(tf);
  } else tf->position--;
  ungetc(tf->buffer[tf->size - 1], tf->fptr);
  for (int i = tf->size - 1; i > 0; i--) {
    tf->buffer[i] = tf->buffer[i - 1];
  }
  tf->buffer[0] = c;
}
