#ifndef RAW_UTILS
#define RAW_UTILS

#include <strings.h>

typedef enum rawtype {
  RAW_INVALID = 0,
  RAW_BOOL,
  RAW_BYTE,
  RAW_UBYTE,
  RAW_SHORT,
  RAW_USHORT,
  RAW_INT,
  RAW_UINT,
  RAW_LONG,
  RAW_ULONG,
  RAW_CHAR,
  RAW_MINI,
  RAW_FLOAT,
  RAW_DOUBLE
} RawType;

typedef struct parsedinteger {
  long integer;
  int  valid;
} ParsedInteger;

typedef struct parseddecimal {
  double decimal;
  int    valid;
} ParsedDecimal;

typedef struct parsedcharacter {
  char character;
  int  valid;
} ParsedCharacter;

ParsedInteger parseinteger(String*);
double parsedecimal(String*);
char   parsecharacter(String*);
#endif