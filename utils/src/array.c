#include <array.h>

#include <string.h>

Array *newArray(size_t element_size)
{
  Array *array = malloc(sizeof(Array));
  void  *content = malloc(element_size);
  if (array != NULL && content != NULL) {
    array->content      = content;
    array->element_size = element_size;
    array->size         = 0;
    array->capacity     = 1;
  } else {
    if (array   == NULL) free(array);
    if (content == NULL) free(content);
    array = NULL;
  }
  return array;
}

void deleteArray(Array **array)
{
  if (*array != NULL) {
    void *content = (*array)->content;
    if (content != NULL) {
      free(content);
    }
    free(*array);
    *array = NULL;
  }
}

int resize(Array *array, int new_size)
{
  int success = 0;
  if (array->size <= new_size) {
    void *tmp = realloc(array->content, new_size * array->element_size);
    if (tmp != NULL) {
      array->content  = tmp;
      array->capacity = new_size;
      success = 1;
    }
  }
  return success;
}

void push(Array *array, void *data)
{
  if (array->size >= array->capacity) {
    void *prevloc = NULL;
    if ((char*)data >= (char*)array->content && 
        (char*)data <  (char*)array->content + (array->element_size * array->size))
    { // array is copying itself, update pointer
      prevloc = array->content;
    }
    if (!resize(array, array->capacity * 2)) return;
    if (prevloc) {
      data = (char*)data + ((long)array->content - (long)prevloc);
    }
  }
  memcpy((char*)array->content + (array->element_size * array->size++),
	       (char*)data,
	       array->element_size);
}

void pushobj(Array *array, void *data) {
  push(array, data);
  free(data);
}

void *pop(Array *array)
{
  void *index = NULL;
  if (array->size > 0) {
    int size     = array->size--;
    int capacity = array->capacity;
    index = (char*)array->content + (array->size * array->element_size);
    if (size > 1 && size < capacity / 4) {
      resize(array, capacity / 2);
    }
  }
  return index;
}

int popobj(Array *array, void(*freefunc)(void*)) {
  void *index = pop(array);
  if (index != NULL) {
    freefunc(index);
    return 1;
  }
  return 0;
}

void *at(Array *array, int index)
{
  if (index >= 0 && index < array->size) {
    return (char*)array->content + (index * array->element_size);
  }
  return NULL;
}

void *last(Array *array)
{
  if (array->size) return (char*)array->content + ((array->size - 1) * array->element_size);
  return NULL;
}

void *rem(Array *array, int index)
{
  void *rem = NULL;
  if (index >= 0 && index < array->size) {
    void *tmp = malloc(array->element_size);
    if (tmp) {
      memcpy(tmp, (char*)array->content + (index * array->element_size), array->element_size);
      memcpy((char*)array->content + (index       * array->element_size),
             (char*)array->content + ((index + 1) * array->element_size),
             (array->size - index) * array->element_size);
      rem = (char*)array->content + (--array->size * array->element_size);
      memcpy(rem, tmp, array->element_size);
      free(tmp);
    }
  }
  return rem;
}

void set(Array *array, int index, void *value)
{
  if (index >= 0 && index < array->size) {
    memcpy((char*)array->content + (index * array->element_size), value, array->element_size);
  }
}

void clear(Array *array)
{
  array->size = 0;
}

void combine(Array* a, Array *b)
{
  void *elem;
  if (a->element_size == b->element_size) {
    while ((elem = pop(b))) {
      push(a, b);
    }
  }
  deleteArray(&b);
}

void insert(Array *array, int index, void *data)
{
  if (array->size >= array->capacity) {
    void *prevloc = NULL;
    if ((char*)data >= (char*)array->content && 
        (char*)data <  (char*)array->content + (array->element_size * array->size))
    { // array is copying itself, update pointer
      prevloc = array->content;
    }
    if (!resize(array, array->capacity * 2)) return;
    if (prevloc) {
      data = (char*)data + ((long)array->content - (long)prevloc);
    }
  }
  // Array is copying the moving part
  if ((char*)data >= (char*)array->content + index       * array->element_size &&
      (char*)data <  (char*)array->content + array->size * array->element_size) {
    data = (char*)data + array->element_size;
  }
  memcpy((char*)array->content + (index + 1) * array->element_size, 
         (char*)array->content +  index      * array->element_size,
         (array->size++        -  index    ) * array->element_size);
  memcpy((char*)array->content +  index      * array->element_size, 
         (char*)data,                          array->element_size);
}

void *in(Array *array, void *data)
{
  void *contains = NULL;
  for (int i = 0; i < array->size; i++) {
    void *tmp = (char*)array->content + i * array->element_size;
    if(!memcmp((char*)data, tmp, array->element_size)) {
      contains = tmp;
      break;
    }
  }
  return contains;
}

int indexof(Array *array, void *data)
{
  int index = -1;
  for (int i = 0; i < array->size; i++) {
    if(!memcmp((char*)data, (char*)array->content + i * array->element_size, array->element_size)) {
      index = i;
      break;
    }
  }
  return index;
}