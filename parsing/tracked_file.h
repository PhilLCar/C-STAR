#ifndef TRACKED_FILE_PARSING
#define TRACKED_FILE_PARSING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct trackedFile {
  FILE *fptr;
  char *buffer;
  int   size;
  int   line;
  int   position;
} TrackedFile;

TrackedFile *tfopen(char*, int);
void         tfclose(TrackedFile*);
char         tfgetc(TrackedFile*);
void         tfungetc(TrackedFile*, char c);

#endif
