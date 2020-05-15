#ifndef STRINGS_UTILS
#define STRINGS_UTILS

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <diagnostic.h>

typedef struct string {
  char *content;
  int   length;
} String;

typedef struct stringstream {
  String *str;
  int     pos;
} StringStream;

String *newString(char*);
void    deleteString(String**);
void    freestring(String*);
String *concat(String*, String*);
String *append(String*, char);
String *prepend(String*, char);
String *inject(String*, int, char);
String *substring(String*, int, int);
String *trim(String*);
int     equals(String*, String*);
int     contains(String*, String*);

StringStream *sopen(String*);
void          sclose(StringStream*);
char          sgetc(StringStream*);
void          sungetc(char c, StringStream*);

#endif