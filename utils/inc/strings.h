#ifndef STRINGS_UTILS
#define STRINGS_UTILS

#include <string.h>
#include <stdlib.h>

typedef struct string {
  char *content;
  int   length;
} String;

String *newString(char*);
void    deleteString(String**);
String *concat(String*, String*);


#endif