#include <file.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <osal.h>

////////////////////////////////////////////////////////////////////////////////
char *filenamewoext(char *filename)
{
  int   len = (int)strlen(filename);
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

////////////////////////////////////////////////////////////////////////////////
char *fileext(char *filename)
{
  char *ext = filename;
  while (ext[0] && ext[0] != '.') ext++;
  return ext;
}

////////////////////////////////////////////////////////////////////////////////
char *filenamewopath(char *filename)
{
  char *last = filename;
  
  for (int i = 0; filename[i]; i++) {
    if (filename[i] == PATH_MARKER) last = &filename[i + 1];
  }
  return last;
}

////////////////////////////////////////////////////////////////////////////////
char *filepath(char *filename)
{
  char *path = NULL;
  int   size = 0;
  
  for (int i = 0; filename[i]; i++) {
    if (filename[i] == PATH_MARKER) size = i + 1;
  }
  path = malloc((size + 1) * sizeof(char));
  if (path) {
    memcpy(path, filename, size);
    path[size] = 0;
  }
  return path;
}

#ifdef WIN

#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////
int fileexists(char *filename, FilePermission permission) {
   WIN32_FIND_DATA file;
   HANDLE handle = FindFirstFile(filename, &file) ;
   int found = handle != INVALID_HANDLE_VALUE;
   if(found) 
   {
       FindClose(handle);
   }
   // For now ignore permission on windows
   permission;
   return found;
}
#else

#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
int fileexists(char *filename, FilePermission permission) {
  int a = 0;

  switch (permission) {
  case FILE_EXISTS:
    a = F_OK;
    break;
  case FILE_READ:
    a = R_OK;
    break;
  case FILE_WRITE:
    a = W_OK;
    break;
  case FILE_EXECUTE:
    a = X_OK;
    break;
  }

  return !access(filename, a);
}
#endif

////////////////////////////////////////////////////////////////////////////////
Stream *fromFileStream(void *stream)
{
  Stream *s = NULL;
  
  if (stream) {
    s = malloc(sizeof(Stream));
    if (s) {
      s->stream = stream;
      s->getc   = (char (*)(void*))fgetc;
      s->ungetc = (void (*)(char, void*))ungetc;
      s->close  = (void (*)(void*))fclose;
    }
  }

  return s;
}
