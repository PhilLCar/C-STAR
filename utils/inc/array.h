#ifndef ARRAY_UTILS
#define ARRAY_UTILS

#include <string.h>
#include <stdlib.h>

typedef struct array {
  void   *content;
  size_t  element_size;
  int     size;
  int     capacity;
} Array;

Array *newArray(size_t);
void   deleteArray(Array**);
int    resize(Array*, int);
void   push(Array*, void*);
void  *pop(Array*);
#endif
