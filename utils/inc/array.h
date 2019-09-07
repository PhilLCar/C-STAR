#ifndef ARRAY_UTILS
#define ARRAY_UTILS

#include <string.h>
#include <stdlib.h>

typedef struct array {
  void *content;
  int   element_size;
  int   size;
  int   capacity;
} Array;

Array *newArray(int, int);
void   deleteArray(Array**);
int    resize(Array*, int);
void   push(Array*, void*);
void  *pop(Array*);
#endif
