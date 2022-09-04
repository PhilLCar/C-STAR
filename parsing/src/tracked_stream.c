#include <tracked_stream.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
TrackedStream *tsopen(Stream *stream, int lookahead)
{
  TrackedStream *ts        = malloc(sizeof(TrackedStream));
  char          *buffer    = malloc((lookahead + 1) * sizeof(char));
  Array         *linestack = newArray(sizeof(int));
  
  if (ts && buffer && linestack) {
    memset(buffer, 0, (lookahead + 1) * sizeof(char));
    
    ts->stream    = stream;

    ts->buffer    = buffer;
    ts->line      = -1;
    ts->position  = -1;

    ts->linestack = linestack;
  } else {
    deleteArray(&linestack);
    if (buffer    != NULL) free(buffer);
    if (ts        != NULL) free(ts);
    ts = NULL;
  }
  return ts;
}

////////////////////////////////////////////////////////////////////////////////
void tsclose(TrackedStream *ts)
{
  deleteArray(&ts->linestack);
  if (ts->stream    != NULL) sclose(ts->stream);
  if (ts->buffer    != NULL) free(ts->buffer);
  if (ts            != NULL) free(ts);
}

////////////////////////////////////////////////////////////////////////////////
char tsgetc(TrackedStream *ts)
{
  if (ts->line < 0) {
    for (int i = 0; i < ts->lookahead; i++) {
      char c;
      do { c = sgetc(ts->stream); } while (c == '\r');
      ts->buffer[i] = c;
    }
    ts->line = 0;
  }
  else {
    char c;
    for (int i = 0; i < ts->lookahead - 1; i++) {
      ts->buffer[i] = ts->buffer[i + 1];
    }
    do { c = sgetc(ts->stream); } while (c == '\r');
    ts->buffer[ts->lookahead - 1] = c;
  }
  if (ts->buffer[0] == '\n') {
    ts->line++;
    push(ts->linestack, ts->position);
    ts->position = -1;
  } else ts->position++;
  
  return ts->buffer[0];
}

////////////////////////////////////////////////////////////////////////////////
void tsungetc(char c, TrackedStream *ts)
{
  sungetc(ts->buffer[ts->lookahead - 1], ts->stream);
  for (int i = ts->lookahead - 1; i > 0; i--) {
    ts->buffer[i] = ts->buffer[i - 1];
  }
  
  if (ts->buffer[0] == '\n') {
    ts->line--;
    ts->position = *(int*)pop(ts->linestack);
  } else ts->position--;
  
  ts->buffer[0] = c;
}
