#ifndef TRACKED_FILE_PARSING
#define TRACKED_FILE_PARSING

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

TrackedFile *tfopen(char*, int);
void         tfclose(TrackedFile*);
char         tfgetc(TrackedFile*);
void         tfungetc(char c, TrackedFile*);

#endif
