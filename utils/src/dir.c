#include <dir.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <file.h>
#include <osal.h>

// Check if pattern is present
/******************************************************************************/
int _is_pattern(char *pattern)
{
  int asterisk = 0;

  for (int i = 0; pattern[i]; i++) if (pattern[i] == '*') {
    asterisk  = 1;
    break;
  }

  return asterisk;
}

// Makes sure the path is terminated properly
/******************************************************************************/
char *_slash_terminate(char *path)
{
  int   len     = strlen(path);
  char *st_path = malloc((len + 2) * sizeof(char));

  if (st_path) {
    strcpy(st_path, path);
    
    if (st_path[len - 1] != PATH_MARKER) {
      st_path[len] = PATH_MARKER;
      st_path[len + 1] = 0;
    }
  }

  return st_path;
}

#ifdef WIN

#include <windows.h>

////////////////////////////////////////////////////////////////////////////////
Array *directory(char *dirname)
{
  WIN32_FIND_DATA  file;
  HANDLE           handle  = NULL;
  Array           *results = NULL;
  int              pattern = _is_pattern(dirname);
  char            *dirpath = pattern ? filepath(dirname) : _slash_terminate(dirname);

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
  if (dirpath != dirname) free(dirpath);
  return results;
}

#else

#ifndef __USE_MISC
#define __USE_MISC
#endif

#include <dirent.h>

// Matches a file pattern
/******************************************************************************/
int _match(char *pattern, char *string)
{
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
  int    pattern  = _is_pattern(dirname);
  char  *filename = pattern ? filenamewopath(dirname) : "";
  char  *dirpath  = pattern ? filepath(dirname)       : _slash_terminate(dirname);
  DIR   *dir      = opendir(dirpath);
  struct dirent *d;

  if (dir) {
    results = newArray(sizeof(DirectoryItem));
    while ((d = readdir(dir))) {
      if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")) continue;
      if (!pattern || _match(filename, d->d_name)) {
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

////////////////////////////////////////////////////////////////////////////////
void freedi(DirectoryItem *di)
{
  free(di->name);
}