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

String *concat(String *a, String *b)
{
  char *n = realloc(a->content, (a->length + b->length + 1) * sizeof(char));
  if (n) {
    memcpy(n + a->length, b->content, b->length + 1);
    a->content = n;
  } else {
    deleteString(&a);
  }
  deleteString(&b);
  return a;
}