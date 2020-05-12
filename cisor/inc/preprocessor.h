#include <string.h>
#include <stdlib.h>

#include <generic_parser.h>
#include <symbol.h>
#include <array.h>
#include <diagnostic.h>
#include <error.h>

#define INCLUDE_MAX_DEPTH        128
#define INCLUDE_MAX_FILE_LENGTH 1024

typedef struct ppvar {
  char *name;
  char *value;
} PPVar;

typedef struct ppenv {
  FILE   *output;
  FILE   *metadata;
  Parser *parser;
  Array  *env;
} PPEnv;

void preprocess(char*, Array*);