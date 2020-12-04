#ifndef TRACKED_STRING_PARSING
#define TRACKED_STRING_PARSING

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

TrackedString *tsopen(String *s, int lookahead);
void           tsclose(TrackedString *ts);
char           tsgetc(TrackedString *ts);
void           tsungetc(char c, TrackedString *ts);

#endif
