#ifndef TRACKED_FILE_PARSING
#define TRACKED_FILE_PARSING

#include <stdio.h>

#include <diagnostic.h>

typedef struct trackedfile {
  FILE *fptr;
  char *buffer;
  int   size;
  int   line;
  int   position;
  int  *linestack;
  int   stack_pos;
  int   stack_cap;
} TrackedFile;

TrackedFile *tfopen(char *filename, int lookahead);
void         tfclose(TrackedFile *tf);
char         tfgetc(TrackedFile *tf);
void         tfungetc(char c, TrackedFile *tf);

#endif
