#ifndef FILE_UTILS
#define FILE_UTILS

typedef enum filepermission {
  FILE_EXISTS,
  FILE_READ,
  FILE_WRITE,
  FILE_EXECUTE
} FilePermission;

char *filenamewoext(char *filename);
char *fileext(char *filename);
char *filenamewopath(char *filename);
char *filepath(char *filename);
int   fileexists(char *filename, FilePermission permission);

#endif