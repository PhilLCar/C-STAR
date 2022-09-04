#ifndef ERRORO_UTILS
#define ERRORO_UTILS

#include <stdio.h>

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

void printirmessage(MessageType type, char *metadata, Symbol *symbol, char *message);
void printmacromessage(MessageType type, Array *trace, Array *history, char *message);
void printnodemessage(MessageType type, Array *trace, char *nodename, char *message);
void printsymbolmessage(MessageType type, Array *trace, Symbol *symbol, char *message);
void printfilemessage(MessageType type, Array *trace, char *message);
void printmessage(MessageType type, char *message);
void printsuggest(char *suggest, char *highlight);

#endif
