#ifndef PREPROCESSOR_CISOR
#define PREPROCESSOR_CISOR

#include <string.h>
#include <stdlib.h>

#include <diagnostic.h>
#include <error.h>
#include <array.h>
#include <strings.h>
#include <symbol.h>
#include <macro.h>
#include <bnf.h>
#include <ast.h>

#define INCLUDE_MAX_DEPTH         128
#define INCLUDE_MAX_FILE_LENGTH  1024

typedef struct ppenv {
  FILE    *output;
  FILE    *metadata;
  Parser  *parser;
  Array   *env;
  Array   *stack;
  BNFNode *tree;
} PPEnv;

void preprocess(char*, Array*);

#endif