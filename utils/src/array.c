#include <array.h>

Array *newArray(int element_size, int capacity)
{
  Array *array = malloc(sizeof(Array));
  void  *content = malloc(element_size * capacity);
  if (array != NULL && content != NULL) {
    array->content      = content;
    array->element_size = element_size;
    array->size         = 0;
    array->capacity     = capacity;
  } else {
    if (array   == NULL) free(array);
    if (content == NULL) free(content);
    array = NULL;
  }
  return array;
}

void deleteArray(Array **array)
{
  void *content = (*array)->content;
  if (content != NULL) {
    free(content);
  }
  free(*array);
  *array = NULL;
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

void append(Array *array, void *data)
{
  if (array->size == array->capacity) {
    if (!resize(array, array->capacity * 2)) return;
  }
  memcpy((char*)array->content + (array->element_size * array->size),
	 (char*)data,
	 array->element_size);
  array->size++;
}

void *rmlast(Array *array)
{
  void *index = NULL;
  if (array->size > 0) {
    int size     = array->size--;
    int capacity = array->capacity;
    index = (char*)array->content + (array->size * array->element_size);
    if (size > 1 && size < capacity / 2) {
      resize(array, capacity / 2);
    }
  }
  return index;
}
