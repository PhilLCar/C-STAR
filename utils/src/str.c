#include <str.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
int consstring(struct string *str, const char *cstr)
{
  int success = 0;

  str->length  = (int)strlen(cstr);
  str->content = malloc((str->length + 1) * sizeof(char));
  if (str->content) {
    memcpy(str->content, cstr, str->length + 1);
    success = 1;
  }

  return success;
}

////////////////////////////////////////////////////////////////////////////////
void freestring(struct string *str)
{
  if (str->content) free(str->content);
}

////////////////////////////////////////////////////////////////////////////////
String *newString(const char *cstr)
{
  String *string = malloc(sizeof(String));

  if (string && !consstring(string, cstr)) {
    free(string);
    string = NULL;
  }

  return string;
}

////////////////////////////////////////////////////////////////////////////////
void deleteString(String **string)
{
  if (*string) {
    freestring(*string);
    free(*string);
    *string = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
String *concat(String *a, String *b)
{
  char *n = realloc(a->content, (a->length + b->length + 1) * sizeof(char));
  if (n) {
    memcpy(n + a->length, b->content, b->length + 1);
    a->content = n;
    a->length += b->length;
  } else {
    deleteString(&a);
  }
  deleteString(&b);
  return a;
}

////////////////////////////////////////////////////////////////////////////////
String *append(String *a, char c)
{
  char *n = realloc(a->content, (a->length + 2) * sizeof(char));
  if (n) {
    a->content              = n;
    a->content[a->length]   = c;
    a->content[++a->length] = 0;
  } else {
    deleteString(&a);
  }
  return a;
}

////////////////////////////////////////////////////////////////////////////////
String *prepend(String *a, char c)
{
  char *n = realloc(a->content, (a->length + 2) * sizeof(char));
  if (n) {
    a->content = n;
    for (int i = a->length + 1; i > 0; i--) a->content[i] = a->content[i - 1];
    a->content[0] = c;
  } else {
    deleteString(&a);
  }
  return a;
}

////////////////////////////////////////////////////////////////////////////////
String *inject(String *a, int index, char c)
{
  char *n = realloc(a->content, (a->length + 2) * sizeof(char));
  if (n) {
    a->content = n;
    for (int i = a->length + 1; i > index; i--) a->content[i] = a->content[i - 1];
    a->content[index] = c;
  } else {
    deleteString(&a);
  }
  return a;
}

////////////////////////////////////////////////////////////////////////////////
String *substring(String *a, int start, int length)
{
  char *s;

  if (length <= 0) length = a->length - start + length;
  s = malloc((length + 1) * sizeof(char));
  if (s) {
    for (int i = 0; i < length; i++) {
      s[i] = a->content[start + i];
    }
    s[length] = 0;
    free(a->content);
    a->content = s;
    a->length  = length;
  } else {
    if (s) free(s);
    deleteString(&a);
  }
  return a;
}

////////////////////////////////////////////////////////////////////////////////
String *trim(String *a) {
  int start, length;
  for (start = 0; start < a->length; start++) {
    char c = a->content[start];
    if (c != ' ' && c != '\t') break;
  }
  for (length = a->length - start; length > 0; length--) {
    char c = a->content[start + length - 1];
    if (c != ' ' && c != '\t') break;
  }
  return substring(a, start, length);
}

////////////////////////////////////////////////////////////////////////////////
int equals(String *a, String *b)
{
  return !strcmp(a->content, b->content);
}

////////////////////////////////////////////////////////////////////////////////
int contains(String *a, String *b)
{
  char *acon  = a->content;
  char *bcon  = b->content;
  int   match = 0;
  int   pos   = -1;

  for (int j = 0; j <= a->length - b->length; j++) {
    match = acon[j] == bcon[0];
    for (int i = 1; match && i < b->length; i++) {
      match &= acon[i + j] == bcon[i];
    }
    if (match) {
      pos = j;
      break;
    }
  }
  return pos;
}

////////////////////////////////////////////////////////////////////////////////
Stream *fromStringStream(void *stream)
{
  Stream *s = NULL;
  
  if (stream) {
    s = malloc(sizeof(Stream));

    if (s) {
      s->stream = stream;
      s->getc   = (char( *)(void*))ssgetc;
      s->ungetc = (void (*)(char, void*))ssungetc;
      s->close  = (void (*)(void*))ssclose;
    }
  }

  return s;
}

////////////////////////////////////////////////////////////////////////////////
StringStream *ssopen(String *s)
{
  StringStream *ss = malloc(sizeof(StringStream));

  if (ss) {
    ss->string = newString(s->content);
    ss->pos    = 0;
    ss->eos    = 0;
  }

  return ss;
}

////////////////////////////////////////////////////////////////////////////////
void ssclose(StringStream *ss)
{
  deleteString(&ss->string);
  free(ss);
}

////////////////////////////////////////////////////////////////////////////////
char ssgetc(StringStream *ss)
{
  char c = ss->string->content[ss->pos];

  if (!c) {
    c = EOF;
    ss->eos = 1;
  } else ss->pos++;

  return c;
}

////////////////////////////////////////////////////////////////////////////////
void ssungetc(char c, StringStream *ss)
{
  if (ss->pos > 0) {
    ss->string->content[--ss->pos] = c;
  }
}