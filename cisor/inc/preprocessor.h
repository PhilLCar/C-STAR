#include <string.h>
#include <stdlib.h>

#include <generic_parser.h>
#include <symbol.h>
#include <array.h>
#include <diagnostic.h>
#include <error.h>
#include <strings.h>

#define INCLUDE_MAX_DEPTH        128
#define INCLUDE_MAX_FILE_LENGTH 1024

typedef struct macro {
  char *name;
  char *value;
} Macro;

typedef struct ppenv {
  FILE   *output;
  FILE   *metadata;
  Parser *parser;
  Array  *env;
  Array  *stack;
} PPEnv;

void preprocess(char*, Array*);