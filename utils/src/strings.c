#include <strings.h>

String *newString(char *content)
{
  String *s = malloc(sizeof(String));
  if (s) {
    s->length = strlen(content);
    s->content = malloc((s->length + 1) * sizeof(char));
    if (s->content) {
      memcpy(s->content, content, s->length + 1);
    } else {
      free(s);
      s = NULL;
    }
  }
  return s;
}

void deleteString(String **s)
{
  if (*s) {
    free((*s)->content);
    free(*s);
  }
}

void freestring(String *s)
{
  free(s->content);
}

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

int equal(String *a, String *b)
{
  return !strcmp(a->content, b->content);
}

int contains(String *a, String *b)
{
  char *acon  = a->content;
  char *bcon  = b->content;
  int   match = 0;

  for (int j = 0; j <= a->length - b->length; j++) {
    match = acon[j] == bcon[0];
    for (int i = 1; match && i < b->length; i++) {
      match &= acon[i + j] == bcon[i];
    }
    if (match) break;
  }
  return match;
}

StringStream *sopen(String *str)
{
  StringStream *s = malloc(sizeof(StringStream));
  if (s) {
    s->str = str;
    s->pos = 0;
  }
  return s;
}

void sclose(StringStream *s)
{
  free(s);
}

char sgetc(StringStream *s)
{
  char c = s->str->content[s->pos];
  if (!c) c = EOF;
  else    s->pos++;
  return c;
}

void sungetc(char c, StringStream *s) {
  if (s->pos > 0) {
    s->str->content[--s->pos] = c;
  }
}