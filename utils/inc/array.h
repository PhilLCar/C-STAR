#ifndef ARRAY_UTILS
#define ARRAY_UTILS

#include <stdlib.h>

#include <diagnostic.h>

#define F void(*)(void*)

typedef struct array {
  void   *content;
  size_t  element_size;
  int     size;
  int     capacity;
} Array;

Array *newArray(size_t elementSize);
void   deleteArray(Array **array);
int    resize(Array *array, int newSize);
void   push(Array *array, void *element);
void   pushobj(Array *array, void *object);
void  *pop(Array *array);
int    popobj(Array *array, void(*freeFunction)(void*));
void  *at(Array *array, int index);
void  *rem(Array *array, int index);
void  *last(Array *array);
void   set(Array *array, int index, void *element);
void   clear(Array *array);
void   combine(Array *persist, Array *delete);
void   insert(Array *array, int index, void *element);
void  *in(Array *array, void *element);
int    indexof(Array *array, void *element);
#endif
