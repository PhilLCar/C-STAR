#include <dir.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <file.h>

#ifdef WIN

#include <windows.h>

////////////////////////////////////////////////////////////////////////////////
Array *directory(char *dirname)
{
  WIN32_FIND_DATA  file;
  HANDLE           handle  = NULL;
  Array           *results = NULL;
  char            *dirpath = filepath(dirname);

  if ((handle = FindFirstFile(dirname, &file)) == INVALID_HANDLE_VALUE) {
    return NULL;
  } else {
    results = newArray(sizeof(DirectoryItem));
  }

  do {
    DirectoryItem di = {
      file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? DIRITEM_DIRECTORY : DIRITEM_FILE,
      malloc((strlen(dirpath) + strlen(file.cFileName) + 1) * sizeof(char))
    };
    sprintf(di.name, "%s%s", dirpath, file.cFileName);
    push(results, &di);
  }
  while(FindNextFile(handle, &file));

  FindClose(handle);
  free(dirpath);
  return results;
}

#else

#ifndef __USE_MISC
#define __USE_MISC
#endif

#include <dirent.h>

////////////////////////////////////////////////////////////////////////////////
int match(char *pattern, char *string) {
  int i, j;
  for (i = 0, j = 0; pattern[i] && string[j]; j++) {
    if (pattern[i] == '*') {
      if (pattern[i + 1] == string[j + 1]) i++;
    } 
    else if (pattern[i] != string[j]) return 0;
    else i++;
  }
  return !pattern[i];
}

////////////////////////////////////////////////////////////////////////////////
Array *directory(char *dirname)
{
  Array *results  = NULL;
  char  *dirpath  = filepath(dirname);
  char  *filename = filenamewopath(dirname);
  DIR   *dir      = opendir(dirpath);
  struct dirent *d;

  if (dir) {
    results = newArray(sizeof(DirectoryItem));
    while ((d = readdir(dir))) {
      if (match(filename, d->d_name)) {
        DirectoryItem di = {
          d->d_type == DT_DIR ? DIRITEM_DIRECTORY : d->d_type == DT_REG ? DIRITEM_FILE : DIRITEM_OTHER,
          malloc((strlen(dirpath) + strlen(d->d_name) + 1) * sizeof(char))
        };
        sprintf(di.name, "%s%s", dirpath, d->d_name);
        push(results, &di);
      }
    }
    closedir(dir);
  }

  free(dirpath);
  return results;
}
#endif

void freedi(DirectoryItem *di)
{
  free(di->name);
}