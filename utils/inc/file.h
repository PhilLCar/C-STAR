#ifndef FILE_UTILS
#define FILE_UTILS

#include <stream.h>

// Enumeration of file permission types
typedef enum file_permission {
  FILE_EXISTS,
  FILE_READ,
  FILE_WRITE,
  FILE_EXECUTE
} FilePermission;

// RETURNS the <filename> without the extension
char *filenamewoext(char *filename);

// RETURNS the <filename>'s extention
char *fileext(char *filename);

// RETURNS the <filename> without the path
char *filenamewopath(char *filename);

// RETURNS the path of the <filename>
char *filepath(char *filename);

// RETURNS 1 if the <filename> exists with the specified <permission>, 0 otherwise
int   fileexists(char *filename, FilePermission permission);

// Gets the abstract stream from file stream
fromStream(File);

#endif