#ifndef ARRAY_UTILS
#define ARRAY_UTILS

#include <stdlib.h>

#include <diagnostic.h>
#include <oop.h>

// Dynamically sized array
typedef struct array {
  void   *content;
  size_t  element_size;
  int     size;
  int     capacity;
} Array;

buildable(Array, array, size_t element_size);

// Resize the <array> to the specified <newSize>
// RETURNS 1 if successful, 0 otherwise
int    resize(Array *array, int newSize);

// Copies a new <element> onto the <array>, (keeps reference)
void   push(Array *array, void *element);

// Moves an <object> onto the <array>, (deletes reference)
void   pushobj(Array *array, void *object);

// Pops an element from the <array>
// RETURNS a pointer to it
void  *pop(Array *array);

// Uses <freefunc> to free the object just removed from the <array>
// RETURNS 1 if successful, 0 otherwise
int    popobj(Array *array, void(*freefunc)(void*));

// RETURNS a pointer to the element at the specified <index>
void  *at(Array *array, int index);

// Removes the element at <index>
// RETURNS a pointer to a copy of it
void  *rem(Array *array, int index);

// RETURNS the last element of the <array>
void  *last(Array *array);

// Sets the <element> at the specified <index>
void   set(Array *array, int index, void *element);

// Clear all data in the <array>
void   clear(Array *array);

// Adds the <delete> array to the back of the <persist> one
void   combine(Array *persist, Array *delete);

// Insert an <element> at the specified <index>
void   insert(Array *array, int index, void *element);

// RETURNS a pointer to the <element> if present, NULL otherwise
void  *in(Array *array, void *element);

// RETURNS the index of the <element> if present, -1 otherwise
int    indexof(Array *array, void *element);

#endif
