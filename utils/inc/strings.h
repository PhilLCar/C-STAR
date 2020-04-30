#ifndef STRINGS_UTILS
#define STRINGS_UTILS

#include <string.h>
#include <stdlib.h>

#include <diagnostic.h>

typedef struct string {
  char *content;
  int   length;
} String;

String *newString(char*);
void    deleteString(String**);
void    freestring(String*);
String *concat(String*, String*);
String *append(String*, char);
int     equal(String*, String*);
int     contains(String*, String*);

#endif