#include <tracked_file.h>

void tfpush(TrackedFile *tf)
{
  if (tf->linestack) {
    tf->linestack[tf->stack_pos++] = tf->position;
    if (tf->stack_pos == tf->stack_cap) {
      int *t = realloc(tf->linestack, (tf->stack_cap *= 2) * sizeof(int));
      if (t == NULL) free(tf->linestack);
      tf->linestack = t;
    }
  }
}

int tfpop(TrackedFile *tf)
{
  int r = 0;
  if (tf->linestack) {
    r = tf->linestack[--tf->stack_pos];
  }
  return r;
}

TrackedFile *tfopen(char *filename, int size)
{
  FILE        *fptr   = fopen(filename, "r");
  char        *buffer = malloc((size + 1) * sizeof(char));
  int          cap    = 16;
  int         *stack  = malloc(cap * sizeof(int));
  TrackedFile *tf     = malloc(sizeof(TrackedFile));
  
  if (fptr && buffer && stack && tf && size) {
    memset(buffer, 0, (size + 1) * sizeof(char));
    
    tf->fptr     = fptr;
    tf->buffer   = buffer;
    tf->size     = size;
    tf->line     = -1;
    tf->position = -1;

    tf->linestack = stack;
    tf->stack_cap = cap;
    tf->stack_pos = 0;
  } else {
    if (fptr   != NULL) fclose(fptr);
    if (buffer != NULL) free(buffer);
    if (stack  != NULL) free(stack);
    if (tf     != NULL) free(tf);
    tf = NULL;
  }
  return tf;
}

void tfclose(TrackedFile *tf)
{
  if (tf->fptr       != NULL) fclose(tf->fptr);
  if (tf->buffer     != NULL) free(tf->buffer);
  if (tf->linestack  != NULL) free(tf->linestack);
  if (tf             != NULL) free(tf);
}

char tfgetc(TrackedFile *tf)
{
  if (tf->line < 0) {
    for (int i = 0; i < tf->size; i++) {
      tf->buffer[i] = fgetc(tf->fptr);
    }
    tf->line = 0;
  }
  else {
    for (int i = 0; i < tf->size - 1; i++) {
      tf->buffer[i] = tf->buffer[i + 1];
    }
    tf->buffer[tf->size - 1] = fgetc(tf->fptr);
  }
  if (tf->buffer[0] == '\n') {
    tf->line++;
    tfpush(tf);
    tf->position = -1;
  } else tf->position++;
  
  return tf->buffer[0];
}

void tfungetc(char c, TrackedFile *tf)
{
  ungetc(tf->buffer[tf->size - 1], tf->fptr);
  for (int i = tf->size - 1; i > 0; i--) {
    tf->buffer[i] = tf->buffer[i - 1];
  }
  
  if (tf->buffer[0] == '\n') {
    tf->line--;
    tf->position = tfpop(tf);
  } else tf->position--;
  
  tf->buffer[0] = c;
}
