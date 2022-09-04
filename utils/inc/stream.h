#ifndef STREAM_UTILS
#define STREAM_UTILS

#include <array.h>
#include <diagnostic.h>
#include <strings.h>

#define streamable(TYPE, type) typedef struct type ## _stream {\
  TYPE *type;\
  int   pos;\
  int   eos;\
} TYPE ## Stream

#define fromStream(TYPE) Stream *from ## TYPE ## Stream(void *stream)

// Abstract stream
typedef struct stream {
  void  *stream;
  char (*getc)(void*);
  void (*ungetc)(char, void*);
  void (*close)(void*);
} Stream;

// RETURNS the next char from the stream <s>
char sgetc(Stream *s);

// Puts the char <c> back on stream <s>
void sungetc(char c, Stream *s);

// Closes the stream <s>
void sclose(Stream *s);

// RETURNS a line form the stream <s>
String *sgetl(Stream *s);

// RETURNS an array from the stream <s>
Array *sgeta(Stream *s);

// Ignores the next line on the stream <s>
void sskipl(Stream *s);

#endif