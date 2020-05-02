#ifndef ARRAY_UTILS
#define ARRAY_UTILS

#include <string.h>
#include <stdlib.h>

#include <diagnostic.h>

#define F void(*)(void*)

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
void   pushobj(Array*, void*);
void  *pop(Array*);
int    popobj(Array*, void(*)(void*));
void  *at(Array*, int);
void  *rem(Array*, int);
void  *last(Array*);
void   set(Array*, int, void*);
void   clear(Array*);
void   combine(Array*, Array*);
void   insert(Array*, int, void*);
#endif
