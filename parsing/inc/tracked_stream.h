#ifndef TRACKED_STREAM_PARSING
#define TRACKED_STREAM_PARSING

#include <stdio.h>

#include <array.h>
#include <diagnostic.h>
#include <stream.h>

// Stream to keep track of line numbers and position on line
typedef struct tracked_stream {
  Stream *stream;
  char   *buffer;
  int     lookahead;
  int     line;
  int     position;
  Array  *linestack;
} TrackedStream;

// RETURNS a new tracked stream from a <stream> with <lookahead> characters of buffer
TrackedStream *tsopen(Stream *stream, int lookahead);

// Closes the tracked stream
void           tsclose(TrackedStream *ts);

// RETURNS a character from the tracked stream <ts>
char           tsgetc(TrackedStream *ts);

// Puts back a character <c> on the tracked stream <ts>
void           tsungetc(char c, TrackedStream *ts);

#endif
