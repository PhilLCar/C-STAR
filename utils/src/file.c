#include <file.h>

#include <stdlib.h>
#include <string.h>

char *filenamewoext(char *filename)
{
  int   len = strlen(filename);
  char *woext = NULL;
  for (int i = len - 1; i >= 0; i--) {
    if (filename[i] == '.') {
      woext = malloc((i + 1) * sizeof(char));
      if (woext) {
        for (int j = 0; j < i; j++) {
          woext[j] = filename[j];
        }
        woext[i] = 0;
      }
      break;
    }
  }
  if (!woext) {
    woext = malloc((len + 1) * sizeof(char));
    if (woext) {
      memcpy(woext, filename, (len + 1) * sizeof(char));
    }
  }
  return woext;
}

char *fileext(char *filename) {
  char *ext = filename;
  while (ext[0] && ext[0] != '.') ext++;
  return ext;
}

char *filenamewopath(char *filename) {
  char *last = filename;
  #ifdef WIN
  char marker = '\\';
  #else
  char marker = '/';
  #endif
  for (int i = 0; filename[i]; i++) {
    if (filename[i] == marker) last = &filename[i + 1];
  }
  return last;
}

char *filepath(char *filename) {
  char *path = NULL;
  int   size = 0;
  #ifdef WIN
  char marker = '\\';
  #else
  char marker = '/';
  #endif
  for (int i = 0; filename[i]; i++) {
    if (filename[i] == marker) size = i + 1;
  }
  path = malloc((size + 1) * sizeof(char));
  memcpy(path, filename, size);
  path[size] = 0;
  return path;
}