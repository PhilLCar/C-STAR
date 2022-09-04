/*
THIS FILE HAS TO BE INCLUDED IN EACH
TRANSLATION UNIT, OR IT WILL NOT WORK
*/
#ifdef  MEMORY_WATCH
#ifndef UTILS_DIAGNOSTIC
#define UTILS_DIAGNOSTIC

#include <stdlib.h>
#include <stdio.h>

// This has the purpose of replacing normal memory management functions
// with these ones that track all allocated pointers, this allows to easily
// determine if the program has a memory leak.

#ifndef UTILS_DIAGNOSTIC_INC
#define  malloc(SIZE_T)       __malloc(SIZE_T, __FILE__, __LINE__)
#define  free(PTR)            __free(PTR)
#define  realloc(PTR, SIZE_T) __realloc(PTR, SIZE_T, __FILE__, __LINE__)
#define  CHECK_MEMORY  printf("Memory usage: %ld bytes\n", __memuse())
#define  STOP_WATCHING __end();
#endif

void   *__malloc(size_t, char*, int);
void    __free(void*);
void   *__realloc(void*, size_t, char*, int);

// RETURNS the current size of allocated memory
size_t  __memuse();

// Prints a report for each unfreed block
void    __end();

#endif
#else
#define CHECK_MEMORY
#define STOP_WATCHING
#endif