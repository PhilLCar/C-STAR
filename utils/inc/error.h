#ifndef ERROR_UTILS
#define ERROR_UTILS

#include <diagnostic.h>
#include <array.h>
#include <symbol.h>

#define CONTEXT_LENGTH 35

typedef enum messagetype {
  ERRLVL_INFO = 0,
  ERRLVL_DEBUG,
  ERRLVL_WARNING,
  ERRLVL_ERROR
} MessageType;

void printmacromessage(MessageType, Array*, Array*, char*);
void printnodemessage(MessageType, Array*, char*, char*);
void printsymbolmessage(MessageType, Array*, Symbol*, char*);
void printfilemessage(MessageType, Array*, char*);
void printmessage(MessageType, char*);
void printsuggest(char*, char*);

#endif
