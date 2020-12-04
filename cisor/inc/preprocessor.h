#ifndef PREPROCESSOR_CISOR
#define PREPROCESSOR_CISOR

#include <stdio.h>

#include <diagnostic.h>
#include <array.h>
#include <bnf.h>
#include <parser.h>
#include <cisor.h>

#define INCLUDE_MAX_DEPTH         128
#define INCLUDE_MAX_FILE_LENGTH  1024

typedef enum pptype {
  PPTYPE_INT,
  PPTYPE_DEC,
  PPTYPE_STRING,
  PPTYPE_CHAR,
  PPTYPE_ERROR
} PPType;

typedef struct ppenv {
  FILE    *output;
  FILE    *metadata;
  Parser  *parser;
  Array   *env;
  Array   *stack;
  BNFNode *tree;
} PPEnv;

typedef struct ppresult {
  PPType  type;
  void   *value;
} PPResult;

void preprocess(Options *options);

#endif