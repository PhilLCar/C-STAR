#ifndef CISOR_CISOR
#define CISOR_CISOR

#include <array.h>

typedef struct {
  int    help;
  int    debug;
  int    version;
  int    preprocess;
  int    compile;
  int    assemble;
  int    link;
  char  *output;
  Array *inputs;
  Array *includepath;
  Array *definitions;
} Options;

#endif