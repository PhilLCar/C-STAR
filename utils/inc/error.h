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

void printmacromessage(MessageType type, Array *trace, Array *history, char *message);
void printnodemessage(MessageType type, Array *trace, char *nodename, char *message);
void printsymbolmessage(MessageType type, Array *trace, Symbol *symbol, char *message);
void printfilemessage(MessageType type, Array *trace, char *message);
void printmessage(MessageType type, char *message);
void printsuggest(char *suggest, char *highlight);

#endif
