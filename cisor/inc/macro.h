#ifndef MACRO_CISOR
#define MACRO_CISOR

#include <strings.h>
#include <array.h>

#define MACRO_NAME_MAX_LENGTH     256
#define MACRO_EXPANSION_MAX_DEPTH 128

typedef enum macroerrors {
  MACRO_VALID = 0,
  MACRO_ERROR_MAX_DEPTH,
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

typedef struct parameter {
  String    *name;
  Expansion *expansion;
} Parameter;

Expansion *newExpansion();
void       deleteExpansion(Expansion**);
void       freemacro(Macro*);
void       freeexpansion(Expansion*);
void       macroexpand(Array*, Parser*, String*, Expansion*, Array*, int);

#endif