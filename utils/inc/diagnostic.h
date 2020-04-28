/*
THIS FILE HAS TO BE INCLUDED IN EACH
TRANSLATION UNIT, OR IT WILL NOT WORK
*/
#ifdef  MEMORY_WATCH
#ifndef UTILS_DIAGNOSTIC
#define UTILS_DIAGNOSTIC

#include <stdlib.h>

#ifndef UTILS_DIAGNOSTIC_INC
#define  malloc       __malloc
#define  free         __free
#define  realloc      __realloc
#define  CHECK_MEMORY printf("Memory usage: %ld bytes\n", memuse())
#endif

void   *__malloc(size_t);
void    __free(void*);
void   *__realloc(void*, size_t) ;
size_t  memuse();

#endif
#else
#define CHECK_MEMORY
#endif