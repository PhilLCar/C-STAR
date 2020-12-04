#ifndef STRINGS_UTILS
#define STRINGS_UTILS

#include <diagnostic.h>

typedef struct string {
  char *content;
  int   length;
} String;

typedef struct stringstream {
  String *str;
  int     pos;
} StringStream;

String *newString(char *string);
void    deleteString(String **s);
void    freestring(String *s);
String *concat(String *a, String *b);
String *append(String *s, char c);
String *prepend(String *s, char c);
String *inject(String *s, int index, char c);
String *substring(String *s, int start, int length);
String *trim(String *s);
int     equals(String *a, String *b);
int     contains(String *a, String *b);

StringStream *sopen(String *s);
void          sclose(StringStream *ss);
char          sgetc(StringStream *ss);
void          sungetc(char c, StringStream *ss);

#endif