#include <tracked_string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tspush(TrackedString *ts)
{
  if (ts->linestack) {
    ts->linestack[ts->stack_pos++] = ts->position;
    if (ts->stack_pos == ts->stack_cap) {
      int *t = realloc(ts->linestack, (ts->stack_cap *= 2) * sizeof(int));
      if (t == NULL) free(ts->linestack);
      ts->linestack = t;
    }
  }
}

int tspop(TrackedString *ts)
{
  int r = 0;
  if (ts->linestack) {
    r = ts->linestack[--ts->stack_pos];
  }
  return r;
}

TrackedString *tsopen(String *str, int size)
{
  StringStream  *sptr   = sopen(str);
  char          *buffer = malloc((size + 1) * sizeof(char));
  int            cap    = 16;
  int           *stack  = malloc(cap * sizeof(int));
  TrackedString *ts     = malloc(sizeof(TrackedString));
  
  if (sptr && buffer && stack && ts && size) {
    memset(buffer, 0, (size + 1) * sizeof(char));
    
    ts->sptr     = sptr;
    ts->buffer   = buffer;
    ts->size     = size;
    ts->line     = -1;
    ts->position = -1;

    ts->linestack = stack;
    ts->stack_cap = cap;
    ts->stack_pos = 0;
  } else {
    if (str    != NULL) sclose(sptr);
    if (buffer != NULL) free(buffer);
    if (stack  != NULL) free(stack);
    if (ts     != NULL) free(ts);
    ts = NULL;
  }
  return ts;
}

void tsclose(TrackedString *ts)
{
  if (ts->sptr       != NULL) sclose(ts->sptr);
  if (ts->buffer     != NULL) free(ts->buffer);
  if (ts->linestack  != NULL) free(ts->linestack);
  if (ts             != NULL) free(ts);
}

char tsgetc(TrackedString *ts)
{
  if (ts->line < 0) {
    for (int i = 0; i < ts->size; i++) {
      char c;
      do { c = sgetc(ts->sptr); } while (c == '\r');
      ts->buffer[i] = c;
    }
    ts->line = 0;
  }
  else {
    char c;
    for (int i = 0; i < ts->size - 1; i++) {
      ts->buffer[i] = ts->buffer[i + 1];
    }
    do { c = sgetc(ts->sptr); } while (c == '\r');
    ts->buffer[ts->size - 1] = c;
  }
  if (ts->buffer[0] == '\n') {
    ts->line++;
    tspush(ts);
    ts->position = -1;
  } else ts->position++;
  
  return ts->buffer[0];
}

void tsungetc(char c, TrackedString *ts)
{
  sungetc(ts->buffer[ts->size - 1], ts->sptr);
  for (int i = ts->size - 1; i > 0; i--) {
    ts->buffer[i] = ts->buffer[i - 1];
  }
  
  if (ts->buffer[0] == '\n') {
    ts->line--;
    ts->position = tspop(ts);
  } else ts->position--;
  
  ts->buffer[0] = c;
}
