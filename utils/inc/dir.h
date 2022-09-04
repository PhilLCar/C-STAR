#ifndef DIR_UTILS
#define DIR_UTILS

#include <array.h>

// Enumeration of possible item types in a directory
typedef enum item_type {
  DIRITEM_FILE,
  DIRITEM_DIRECTORY,
  DIRITEM_OTHER
} ItemType;

// Directory item
typedef struct dir_item {
  ItemType  type;
  char     *name;
} DirectoryItem;

// RETURNS an array containng the directory items in the directory sepcified by <dirname>
Array *directory(char *dirname);

// Frees the Directory Item <di>
void   freedi(DirectoryItem *di);

#endif