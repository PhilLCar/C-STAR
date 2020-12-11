#ifndef DIR_UTILS
#define DIR_UTILS

#include <array.h>

typedef enum itemtype {
  DIRITEM_FILE,
  DIRITEM_DIRECTORY,
  DIRITEM_OTHER
} ItemType;

typedef struct diritem {
  ItemType  type;
  char     *name;
} DirectoryItem;

Array *directory(char *dirname);
void   freedi(DirectoryItem *di);

#endif