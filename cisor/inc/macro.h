#ifndef MACRO_CISOR
#define MACRO_CISOR

#include <stdlib.h>

#include <strings.h>
#include <array.h>
#include <symbol.h>

#define MACRO_NAME_MAX_LENGTH     256
#define MACRO_EXPANSION_MAX_DEPTH 128

typedef enum macroerrors {
  MACRO_ERROR_MAX_DEPTH = 1,
  MACRO_ERROR_WRONG_CALL_FORMAT,
  MACRO_ERROR_WRONG_PARAMETER_FORMAT,
  MACRO_ERROR_EOF_IN_PARAMETERS,
  MACRO_ERROR_PARAMETER_MISMATCH
} MacroErrors;

typedef struct macro {
  String *filename;
  String *name;
  String *value;
  Array  *params;
  int     line;
  int     position;
} Macro;

typedef struct expanded {
  Macro *m;
  int    position;
} Expanded;

typedef struct expansion {
  String *value;
  Array  *hist;
  int     invalid;
} Expansion;

Expansion *newExpansion();
void       deleteExpansion(Expansion**);
void       freemacro(Macro*);
void       freeexpansion(Expansion*);
void       macroexpand(Array*, Parser*, String*, Expansion*);

#endif