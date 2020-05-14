#ifndef TRACKED_STRING_PARSING
#define TRACKED_STRING_PARSING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <diagnostic.h>
#include <strings.h>

typedef struct trackedstring {
  StringStream *sptr;
  char         *buffer;
  int           size;
  int           line;
  int           position;
  int          *linestack;
  int           stack_pos;
  int           stack_cap;
} TrackedString;

TrackedString *tsopen(String*, int);
void           tsclose(TrackedString*);
char           tsgetc(TrackedString*);
void           tsungetc(char c, TrackedString*);

#endif
