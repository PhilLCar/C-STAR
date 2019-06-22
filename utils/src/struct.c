#include <struct.h>

void savetofile(char *filename, void *structure, int size) {
  FILE *f = fopen(filename, "w+");
  
  if (f != NULL) {
    for (int i = 0; i < size; i++) {
      fputc(((char*)structure)[i], f);
    }
    
    fclose(f);
  }
}

void *fromfile(char *filename, int size) {
  FILE *f = fopen(filename, "r");
  void *s = malloc(size);

  if (f != NULL && s != NULL) {
    for (int i = 0; i < size; i++) {
      ((char*)s)[i] = fgetc(f);
    }
    
    fclose(f);
  } else {
    if (f) fclose(f);
    if (s) free(s);
    s = NULL;
  }
  
  return s;
}
