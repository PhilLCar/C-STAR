#include <preprocessor.h>

char *filenamewoext(char *filename) {
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

void preprocess(char *filename, Array *includes) {
  char  ppfile[512];
  char  metafile[512];
  char *woext = filenamewoext(filename);
  sprintf(ppfile,   "%s.psr", woext);
  sprintf(metafile, "%s.msr", woext);
  free(woext);
  //////////////////////////////////////////
  TrackedFile *tf   = tfopen(filename, 1);
  FILE        *pp   = fopen(ppfile,   "w+");
  FILE        *meta = fopen(metafile, "w+");

  if (tf)   tfclose(tf);
  if (pp)   fclose(pp);
  if (meta) fclose(meta);
}