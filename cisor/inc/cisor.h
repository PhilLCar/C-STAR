#ifndef CISOR_CISOR
#define CISOR_CISOR

#include <array.h>

typedef struct {
  int    help;
  int    debug;
  int    version;
  int    preprocessed;
  int    assembly;
  int    compiled;
  char  *output;
  Array *inputs;
  Array *includepath;
} Options;

#endif